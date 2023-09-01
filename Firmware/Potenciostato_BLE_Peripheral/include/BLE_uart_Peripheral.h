/**
 * @file BLE_uart_Peripheral.h
 * @brief Arquivo de cabeçalho para o funcionamento do dispositivo BLE como periférico UART.
 */

#ifndef BLE_UART_PERIPHERAL_H
#define BLE_UART_PERIPHERAL_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


/**
 * @brief Tamanho máximo da string esperada para ser enviada.
 */
#define MAX_STRING_LENGTH 510

/**
 * @brief UUID do serviço que será usado para a comunicação UART.
 */
#define SERVICE_UUID "45925276-4809-11ee-be56-0242ac120002" // UART service UUID

/**
 * @brief UUID da característica de recepção UART.
 */
#define CHARACTERISTIC_UUID_RX "45925277-4809-11ee-be56-0242ac120002"

/**
 * @brief UUID da característica de transmissão UART.
 */
#define CHARACTERISTIC_UUID_TX "45925278-4809-11ee-be56-0242ac120002"

/**
 * @brief Inicializa o dispositivo BLE.
 */
void inicia_BLE();

/**
 * @brief Escreve dados no dispositivo BLE.
 * 
 * @param value Valor a ser escrito.
 */
void write_BLE(std::string value);

// void restart_advertising();

/**
 * @class MyCallbacks
 * @brief Classe de retorno de chamada para a característica BLE.
 */
class MyCallbacks : public BLECharacteristicCallbacks
{
public:
    void onWrite(BLECharacteristic *pCharacteristic);
};

/**
 * @class MyServerCallbacks
 * @brief Classe de retorno de chamada para o servidor BLE.
 */
class MyServerCallbacks : public BLEServerCallbacks
{
public:
    void onConnect(BLEServer *pServer);
    void onDisconnect(BLEServer *pServer);
    // void onMtuChanged(BLEServer *pServer, esp_ble_gatts_cb_param_t *param);
};

#endif