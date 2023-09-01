/**
 * @file main.ino
 * @brief Implementação das funções e configurações principais do sistema.
 *
 * Este arquivo contém a implementação das funções setup(), loop() e outras
 * funções relacionadas à configuração do sistema e às tarefas executadas
 * pelo FreeRTOS. Também contém a inicialização de variáveis globais.
 */

// Standard FreeRTOS Libraries
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

// Custom Internal Libraries
#include "main.h"
#include "LMP91000.h"
#include "LMPapp.h"
#include "BLE_uart_Peripheral.h"

// Variáveis externas
extern bool deviceConnected;


/**
 * @brief Função de configuração inicial do sistema.
 *
 * Esta função é chamada uma vez durante a inicialização do sistema e é
 * responsável por configurar os pinos, as filas, os semáforos, e iniciar
 * as tarefas do FreeRTOS.
 */
void setup()
{

  Wire.begin();           // Inicializa o barramento I2C
  Wire.setClock(100000);  // Define a velocidade do I2C para 100 kHz

  Serial.begin(115200);

  // Criando fila
  Metodo_queue = xQueueCreate(1, sizeof(StructControlMetodo));
  cmd_queue = xQueueCreate(10, sizeof(char[MAX_STRING_LENGTH]));
  output_queue = xQueueCreate(400, sizeof(StructControlOutput));

  // Inicia BLE
  inicia_BLE();

  // Inicia dac
  init_dac();

  // Incia ADC
  adcStartup(cs_ads, reset_ads, drdy_ads);

  // Cria Semaphore para configurar LMP
  xSemaphore_writeLMP = xSemaphoreCreateBinary();

  // Inicia LMP         //true loop e false sem loop
  init_lmp(false);      // Irá inicializar o LMP e executar o loop do-while normalmente

  iniciarMetodo();      //Inicia task do Metodo

  xTaskCreate(
      TaskReadSerial, "SerialR", 4096 // Stack size 4096
      ,
      NULL, 2 // Priority
      ,
      NULL); //, 0

  xTaskCreate(
      TaskCmd, "CMD", 4096 // Stack size 4096
      ,
      NULL, 2 // Priority
      ,
      NULL); //, 0

  xTaskCreatePinnedToCore(
      TaskOutput, "OUT", 4096 // Stack size 4096
      ,
      NULL, 2 // Priority
      ,
      NULL, 1); //, 0

  xTaskCreatePinnedToCore(
      TaskBlink, "Blink" // A name just for humans
      ,
      1024 // This stack size can be checked & adjusted by reading the Stack Highwater
      ,
      NULL, 2 // Priority, with 1 being the highest, and 4 being the lowest.
      ,
      NULL, 0);
}

/**
 * @brief Função principal do sistema.
 *
 * Esta função é chamada repetidamente pelo sistema após a configuração
 * inicial. É usada principalmente para lidar com eventos ou tarefas de menor
 * prioridade que não justificam uma tarefa separada.
 */
void loop()
{
  // Vazio, pois as tarefas são tratadas pelo FreeRTOS.
  vTaskDelete(NULL);
}

/**
 * @brief Função da tarefa que lida com a saída de dados.
 *
 * Esta função é responsável por receber os dados da fila de saída e enviá-los
 * para a BLE (Bluetooth Low Energy) ou para a porta serial, dependendo da
 * conectividade.
 *
 * @param pvParameters Parâmetros da tarefa (não utilizado).
 */
void TaskOutput(void *pvParameters)
{
  StructControlOutput output_r;
  String output;

  for (;;)
  {
    if (xQueueReceive(output_queue, &output_r, 100))
    {
      switch (output_r.type)
      {
      case CV_metodo:
        output = String(output_r.output.cv.cycle) + "\t" +
                 String(output_r.output.cv.voltage_f) + "\t" +
                 String(output_r.output.cv.current2, 5) + "\n";

        if (deviceConnected)
        {
          write_BLE(std::string(output.c_str()));
          Serial.print(output.c_str());
        }
        else
        {
          Serial.print(output.c_str());
        }

        break;

      case Amp_metodo:
        output = String(output_r.output.amp.voltage) + "\t" + String(output_r.output.amp.time) +
                 "\t" + String(output_r.output.amp.current, 5) + "\n";

        if (deviceConnected)
        {
          write_BLE(std::string(output.c_str()));
          Serial.print(output.c_str());
        }
        else
        {
          Serial.print(output.c_str());
        }
        break;

      case Generic_type:
        output = (output_r.output.string);

        if (deviceConnected)
        {
          write_BLE(std::string(output.c_str()));
          Serial.print(output.c_str());
        }
        else
        {
          Serial.print(output.c_str());
        }
        break;

      default:
        Serial.print("DEFAULT: ");
        Serial.println(output_r.output.string);
        break;
      }
    }
  }
}

/**
 * @brief Função da tarefa que lida com comandos recebidos.
 *
 * Esta função é responsável por receber comandos da fila de comandos, analisá-los
 * e tomar ações correspondentes, como iniciar um método ou enviar informações
 * pela BLE ou porta serial.
 *
 * @param pvParameters Parâmetros da tarefa (não utilizado).
 */
void TaskCmd(void *pvParameters)
{
  String inputString;
  StructControlOutput output_r;
  int values[50]; // Array para armazenar os valores inteiros

  char cmd_data[MAX_STRING_LENGTH];

  for (;;)
  {
    if (xQueueReceive(cmd_queue, &cmd_data, 100))
    {
      inputString = cmd_data;
      // Serial.println("TaskCmd recebida: " + inputString);
      
      if (inputString == "status_BLE\n")
      {
        Serial.println("uart_only\n");
      }
      if (inputString == "stop" || inputString == "stop\n")
      {
        inicia_blink = false;
        pararMetodo();
      }
      else
      {
        int index = 0;
        String value;
        // Percorre a string e separa os valores com base no delimitador ";"
        for (int i = 0; i < inputString.length(); i++)
        {
          char c = inputString.charAt(i);
          if (c == ';')
          {
            values[index] = value.toInt(); // Converte a substring para inteiro
            index++;
            value = ""; // Limpa a string temporária para o próximo valor
          }
          else
          {
            value += c; // Adiciona o caractere à string temporária
          }
        }

        // Último valor após o último delimitador
        values[index] = value.toInt();

        if (values[0] == 0 && index >= 9)
        { // inicia CV
          iniciarMetodo();
          inicia_blink = true;
          CV_setup mandaCV{values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8], values[9]};
          StructControlMetodo send_metodo;
          send_metodo.type = CV_metodo;
          send_metodo.metodo.cv = mandaCV;
          xQueueSend(Metodo_queue, &send_metodo, 0);
        }
        else if (values[0] == 1 && index >= 10)
        { // inicia Amp
          iniciarMetodo();
          inicia_blink = true;
          Amp_setup mandaAmp{values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8], values[9], values[10]};
          StructControlMetodo send_metodo;
          send_metodo.type = Amp_metodo;
          send_metodo.metodo.amp = mandaAmp;
          xQueueSend(Metodo_queue, &send_metodo, 0);
        }
      }
    }
  }
}

/**
 * @brief Função da tarefa que lida com a leitura da porta serial.
 *
 * Esta função é responsável por ler dados da porta serial, processá-los e
 * enviá-los para a fila de comandos para posterior processamento.
 *
 * @param pvParameters Parâmetros da tarefa (não utilizado).
 */
void TaskReadSerial(void *pvParameters)
{
  String inputString;
  char buffer[MAX_STRING_LENGTH];

  for (;;)
  {

    if (Serial.available() > 0)
    {
      String inputString = Serial.readString(); // Lê a string recebida pela serial
      // Serial.println("TaskReadSerial recebida: " + inputString);

      // Limpar o buffer antes de usar
      memset(buffer, 0, sizeof(buffer));
      // Copiar a string para o buffer
      strncpy(buffer, inputString.c_str(), inputString.length());

      // Enviar o buffer pela fila
      xQueueSend(cmd_queue, &buffer, 0);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Função da tarefa que controla o LED.
 *
 * Esta função é responsável por controlar o LED, alternando seu estado
 * com base na variável inicia_blink.
 *
 * @param pvParameters Parâmetros da tarefa (não utilizado).
 */
void TaskBlink(void *pvParameters)
{
  // inicializa pinos
  pinMode(LED_Status_1, OUTPUT);
  pinMode(LED_Status_2, OUTPUT);
  bool current_blink = false;
  inicia_blink = false;
  int delayTime = 1000;

  for (;;)
  {
    if (current_blink != inicia_blink)
    {
      if (inicia_blink)
      {
        digitalWrite(LED_Status_1, HIGH);
        digitalWrite(LED_Status_2, HIGH);
      }
      else
      {
        digitalWrite(LED_Status_1, HIGH);
        digitalWrite(LED_Status_2, LOW);
      }
      current_blink = inicia_blink;
    }

    if (inicia_blink)
    {
      delayTime = 100;
      digitalWrite(LED_Status_2, !digitalRead(LED_Status_2)); // Toogle LED
      digitalWrite(LED_Status_1, !digitalRead(LED_Status_1)); // Toogle LED
    }
    else
    {
      delayTime = 500;
      digitalWrite(LED_Status_1, HIGH);                       // Toogle LED
      digitalWrite(LED_Status_2, !digitalRead(LED_Status_2)); // Toogle LED
    }
    vTaskDelay(delayTime / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Inicializa o DAC.
 *
 * Esta função é responsável por inicializar as configurações do DAC.
 */
void init_dac()
{
  // Set pin dac
  dac.mosi = mosi_dac;
  //  dac.miso = misoPin;
  dac.sck = clk_dac;
  dac.cs = cs_dac;
  dac.drv_sel = DAC_DRV_SEL_SPI;

  dac_init(&dac);
  dac_soft_reset(&dac);
  dac_set_gain(&dac, DAC_GAIN_REF_DIV_DISABLE, DAC_GAIN_BUFF_GAIN_2);
}

/**
 * @brief Inicializa o LMP (LMP91000).
 *
 * Esta função é responsável por inicializar as configurações do LMP91000.
 *
 * @param bypassWhile Flag para indicar se o loop while deve ser ignorado.
 */
void init_lmp(bool bypassWhile)
{

  xSemaphoreGive(xSemaphore_writeLMP);
  xSemaphoreTake(xSemaphore_writeLMP, portMAX_DELAY);

  config_lmp init_lmp{
      0, // lmpGain
      3, // RLoad
      2, // IntZ
      0  // bias_setting
  };

  // enable the potentiostat
  setupLMP(MENB);
  initLMPapp(init_lmp, &dac);
  setOutputsToZero();

  bool pstat_init = false;
  do
  {
    pstat_init = LMP_isReady();
    if (pstat_init)
      Serial.println(" LMP INICIALIZADO COM SUCESSO! ");
    else
    if (!pstat_init)
    {
      Serial.println(" ERRO: LMP NAO INICIALIZADO ");
      initLMPapp(init_lmp, &dac);
      setOutputsToZero();
    }
    if (!bypassWhile)
    {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  } while (!pstat_init && !bypassWhile);

  xSemaphoreGive(xSemaphore_writeLMP);
}
