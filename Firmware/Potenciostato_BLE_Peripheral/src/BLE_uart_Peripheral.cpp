/**
 * @file BLE_uart_Peripheral.cpp
 * @brief Implementação do dispositivo BLE como periférico UART.
 */

// Standard FreeRTOS Libraries
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "BLE_uart_Peripheral.h"
#include <Arduino.h>


// Variáveis externas
extern xQueueHandle cmd_queue;

/**
 * @brief Indica se um dispositivo está conectado.
 */
bool deviceConnected = false;

/**
 * @brief Ponteiro para a característica BLE.
 */
BLECharacteristic *pCharacteristic;


/**
 * @brief Inicializa o dispositivo BLE.
 */
void inicia_BLE()
{
    // Cria o dispositivo BLE
    BLEDevice::init("UART Service");

    // Cria o servidor BLE
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Cria o serviço BLE
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Cria uma característica BLE
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY);

    pCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setCallbacks(new MyCallbacks());

    // Inicia o serviço
    pService->start();

    // Inicia a publicidade
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

/**
 * @brief Callbacks de conexão do servidor BLE.
 */
void MyServerCallbacks::onConnect(BLEServer *pServer)
{
    deviceConnected = true;
    Serial.println("CONECTADO");
}

/**
 * @brief Callbacks de desconexão do servidor BLE.
 */
void MyServerCallbacks::onDisconnect(BLEServer *pServer)
{
    deviceConnected = false;
    pServer->startAdvertising(); // restart advertising
    Serial.println("DESCONECTADO startAdvertising");
}

// void MyServerCallbacks::onMtuChanged(BLEServer *pServer, esp_ble_gatts_cb_param_t *param)
// {
//     Serial.println("onMtuChanged");
// }

/**
 * @brief Callbacks de escrita da característica BLE.
 */
void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
    std::string rxValue = pCharacteristic->getValue();
    // Serial.print("****Peripherical recebe: ");
    // Serial.println(rxValue.c_str());

    char buffer[MAX_STRING_LENGTH];

    // Limpa o buffer antes de usar
    memset(buffer, 0, sizeof(buffer));
    // Copia a string para o buffer
    strncpy(buffer, rxValue.c_str(), rxValue.length());

    // Envia o buffer para a fila
    xQueueSend(cmd_queue, &buffer, 0);
}

/**
 * @brief Escreve dados no dispositivo BLE.
 * 
 * @param value Valor a ser escrito.
 */
void write_BLE(std::string value)
{
    pCharacteristic->setValue(value.c_str());
    pCharacteristic->notify(); // Send the value to the app!
}