/**
 * @file main.c
 * @brief Arquivo principal contendo a funcionalidade principal.
 */

// Standard FreeRTOS Libraries
#include <Arduino.h>
#include "BLEDevice.h"

// Custom Internal Libraries
#include "main.h"

/**
 * @brief Função de retorno de chamada para notificações.
 * 
 * @param pBLERemoteCharacteristic Ponteiro para a característica remota BLE.
 * @param pData Ponteiro para os dados recebidos.
 * @param length Tamanho dos dados recebidos.
 * @param isNotify Indica se é uma notificação.
 */
static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  char buffer[MAX_STRING_LENGTH];

  // Limpar o buffer antes de usar
  memset(buffer, 0, sizeof(buffer));
  // Copiar a string para o buffer
  strncpy(buffer, (char *)pData, length);

  // Encontrar a posição da primeira ocorrência de '\n'
  char *newlinePos = strchr(buffer, '\n');

  if (newlinePos != nullptr)
  {
    // Calcular o índice onde o '\n' ocorre
    size_t newlineIndex = newlinePos - buffer;

    // Substituir o '\n' por um caractere nulo para terminar a string no ponto desejado
    buffer[newlineIndex] = '\0';
  }

  // Enviar o buffer pela fila
  xQueueSend(output_queue, &buffer, 0);
}

/**
 * @class MyClientCallback
 * @brief Classe de retorno de chamada para o cliente BLE.
 */
class MyClientCallback : public BLEClientCallbacks
{
  /**
   * @brief Chamado quando a conexão é estabelecida.
   * @param pclient Ponteiro para o cliente BLE.
   */
  void onConnect(BLEClient *pclient)
  {
    Serial.println("connect_BLE");
  }

  /**
   * @brief Chamado quando a conexão é interrompida.
   * @param pclient Ponteiro para o cliente BLE.
   */
  void onDisconnect(BLEClient *pclient)
  {
    deviceConnected = false;
    Serial.println("disconnect_BLE");
  }
};

/**
 * @brief Conecta ao servidor BLE com o endereço especificado.
 * 
 * @param pAddress Endereço BLE do servidor.
 * @return true se a conexão for bem-sucedida, false caso contrário.
 */
bool connectToServer(BLEAddress pAddress)
{
  Serial.print("Establishing a connection to device address: ");
  Serial.println(pAddress.toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");

  uint16_t mtu = 510;
  pClient->setMTU(mtu);

  // Obtain a reference to the Nordic UART service on the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find Nordic UART service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Remote BLE service reference established");

  // Obtain a reference to the TX characteristic of the Nordic UART service on the remote BLE server.
  pTXCharacteristic = pRemoteService->getCharacteristic(charUUID_TX);
  if (pTXCharacteristic == nullptr)
  {
    Serial.print("Failed to find TX characteristic UUID: ");
    Serial.println(charUUID_TX.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Remote BLE TX characteristic reference established");

  // Read the value of the TX characteristic.
  if (pTXCharacteristic->canRead())
  {
    std::string value = pTXCharacteristic->readValue();
    Serial.print("The characteristic value is currently: ");
    Serial.println(value.c_str());
  }

  pTXCharacteristic->registerForNotify(notifyCallback);

  // Obtain a reference to the RX characteristic of the Nordic UART service on the remote BLE server.
  pRXCharacteristic = pRemoteService->getCharacteristic(charUUID_RX);
  if (pRXCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_RX.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Remote BLE RX characteristic reference established");

  deviceConnected = true;
  return true;
}

/**
 * @class MyAdvertisedDeviceCallbacks
 * @brief Classe de retorno de chamada para dispositivos BLE anunciados.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  /**
   * @brief Chamada para cada servidor BLE anunciado.
   * @param advertisedDevice Dispositivo BLE anunciado.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found - ");
    Serial.println(advertisedDevice.toString().c_str());

    // Encontramos um dispositivo, verifique se ele contém o serviço personalizado.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID))
    {

      Serial.println("Found a device with the desired ServiceUUID!");
      advertisedDevice.getScan()->stop();

      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true;
      doScan = true;

    } // Found our server
  }   // onResult
};    // MyAdvertisedDeviceCallbacks

/**
 * @brief Configuração do programa.
 */
void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Central Mode (Client) Nordic UART Service");

  BLEDevice::init("Pot UART Central");

  // Recupera um Scanner e define o retorno de chamada para ser informado quando
  // um novo dispositivo é detectado. Especifica que queremos um scan ativo e inicia
  // o scan para rodar por 5 segundos.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);   // Define o intervalo de scan.
  pBLEScan->setWindow(449);      // Define a janela de scan ativo
  pBLEScan->setActiveScan(true); // Realiza um scan ativo
  pBLEScan->start(5, false);

  output_queue = xQueueCreate(400, sizeof(char[MAX_STRING_LENGTH]));

  xTaskCreate(
      TaskReadSerial, "SerialR", 4096 // Tamanho da pilha
      ,NULL, 3 // Prioridade
      ,NULL); //, 0

  xTaskCreatePinnedToCore(
      TaskWriteSerial, "SerialW", 4096 // Tamanho da pilha
      ,NULL, 1 // Prioridade
      ,NULL, 1); // Core utilizado para task

}  // Fim da configuração.

/**
 * @brief Tarefa para lidar com a escrita na porta serial.
 * 
 * @param pvParameters Ponteiro para os parâmetros da tarefa (não utilizado).
 */
void TaskWriteSerial(void *pvParameters)
{
  char output_data[MAX_STRING_LENGTH];

  for (;;)
  {
    //* Esta fila é preenchida pela função de callback de notificação BLE
    if (xQueueReceive(output_queue, &output_data, 100))
    {
      if (deviceConnected)
      {
        std::string receivedMessage = output_data;
        // Serial.print("****Central recebe: ");
        Serial.println(receivedMessage.c_str());
        // Serial.println(receivedMessage.size());
      }
      else
      {
        Serial.println("disconnect_BLE");
      }
    }
    // vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Tarefa para ler dados da porta serial e enviar para o dispositivo BLE.
 * 
 * @param pvParameters Ponteiro para os parâmetros da tarefa (não utilizado).
 */
void TaskReadSerial(void *pvParameters)
{
  String inputString = "";     // Uma String para armazenar os dados recebidos
  bool stringComplete = false; // indica se a string está completa
  for (;;)
  {
    if (Serial.available() > 0)
    {
      inputString = Serial.readStringUntil('\n');
      inputString.trim();
      
      if(inputString == "status_BLE"){
        if (deviceConnected) Serial.println("connect_BLE");
        else Serial.println("disconnect_BLE");        
      }
      else if (deviceConnected)
      {
        pRXCharacteristic->writeValue(inputString.c_str(), inputString.length());
        // Serial.print("****Central envia: ");
        // Serial.println(inputString.c_str());
        // Serial.print("Tamanho: ");
        // Serial.println(strlen(inputString.c_str()));
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Função principal do loop de execução.
 */
void loop()
{
  // Se a flag "doConnect" for verdadeira, escaneamos e encontramos o servidor BLE desejado.
  // Agora nos conectamos a ele. Uma vez conectados, definimos a flag de conexão como verdadeira.
  if (doConnect == true)
  {
    if (connectToServer(*pServerAddress))
    {
      Serial.println("We are now connected to the BLE Server.");
    }
    else
    {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  if (!deviceConnected)
  {
    BLEDevice::getScan()->start(0);  //faz o scaneamento novamente se o dispositivo estiver desconectado. 
  }
  vTaskDelay(500 / portTICK_PERIOD_MS);
} // Fim do loop
