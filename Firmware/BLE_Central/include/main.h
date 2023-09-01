/**
 * @file main.h
 * @brief Arquivo de cabeçalho para a funcionalidade principal (main.c).
 */

#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include "BLEDevice.h"


/**
 * @brief Tamanho máximo da string esperada para ser enviada.
 */
#define MAX_STRING_LENGTH 100

/**
 * @brief UUID do serviço ao qual deseja conectar.
 * 
 * @details Esse serviço expõe duas características: uma para transmissão e outra para recepção (como visto pelo cliente).
 */
static BLEUUID serviceUUID("45925276-4809-11ee-be56-0242ac120002");

/**
 * @brief UUID características do serviço para o de RX para receber dados.
 */
static BLEUUID charUUID_RX("45925277-4809-11ee-be56-0242ac120002");

/**
 * @brief UUID características do serviço para o de TX para enviar dados.
 */
static BLEUUID charUUID_TX("45925278-4809-11ee-be56-0242ac120002");

/**
 * @brief Fila de saída de dados BLE.
 */
static QueueHandle_t output_queue = NULL;

/**
 * @brief Tarefa para ler comandos que vem da serial.
 * 
 * @param pvParameters 
 */
void TaskReadSerial(void *pvParameters);

/**
 * @brief Tarefa para escrever em série.
 * 
 * @param pvParameters 
 */
void TaskWriteSerial(void *pvParameters);


/**
 * @brief Ponteiro para o endereço BLE do servidor.
 */
static BLEAddress *pServerAddress;

/**
 * @brief Flag para indicar se deve iniciar a conexão.
 */
static boolean doConnect = false;

/**
 * @brief Flag para indicar se um dispositivo está conectado.
 */
static boolean deviceConnected = false;

/**
 * @brief Flag para indicar se deve iniciar a varredura (scanning).
 */
static boolean doScan = false;

/**
 * @brief Ponteiro para a característica remota para transmissão.
 */
static BLERemoteCharacteristic *pTXCharacteristic;

/**
 * @brief Ponteiro para a característica remota para recepção.
 */
static BLERemoteCharacteristic *pRXCharacteristic;

#endif
