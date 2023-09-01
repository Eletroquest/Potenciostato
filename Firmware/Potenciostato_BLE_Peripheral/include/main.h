/**
 * @file main.h
 * @brief Declaração de variáveis e funções para um sistema FreeRTOS.
 *
 * Este arquivo contém as declarações de variáveis e funções para um sistema
 * que utiliza o FreeRTOS. As variáveis relacionadas ao FreeRTOS são separadas
 * das demais.
 */

#ifndef MAIN_H
#define MAIN_H

//Standard Arduino Libraries
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// Custom Internal Libraries
#include "ads131a04.h"
#include "dac80501.h"


/** @defgroup FreeRTOS_Variables Variáveis do FreeRTOS
 * @{
 */

SemaphoreHandle_t xSemaphore_writeLMP; ///< Semáforo para escrita LMP
TaskHandle_t taskMetodo_Handle;        ///< Handle da tarefa do método

xQueueHandle Metodo_queue; ///< Fila para o método

/** @} */

/** @defgroup Other_Variables Outras Variáveis
 * @{
 */

bool inicia_blink; ///< Flag para iniciar o LED
xQueueHandle cmd_queue; ///< Fila para comandos
xQueueHandle output_queue; ///< Fila para saída de dados
static dac_t dac; ///< Estrutura de controle do DAC

/** @defgroup Pin_Definitions Definições de Pinos
 * @{
 */

// Pinos ADS
uint16_t drdy_ads = 27;
uint16_t reset_ads = 25;
uint16_t cs_ads = 5;
uint16_t miso_ads = 19;
uint16_t mosi_ads = 23;
uint16_t clk_ads = 18;

// Pinos DAC
//uint16_t csPin = 15;
uint16_t cs_dac = 33;
uint16_t miso_dac = 19;
uint16_t mosi_dac = 23;
uint16_t clk_dac = 18;

// Pinos LED
int LED_Status_1 = 2;
int LED_Status_2 = 16;

// Pinos LMP91000 - control pins
const uint8_t MENB = 26; ///< MENB
const uint8_t LMP_C1 = 33; ///< LMP_C1
const uint8_t LMP_C2 = 32; ///< LMP_C2
const uint8_t LMP = 35; ///< LMP
const uint8_t anti_aliased = 34; ///< Anti-aliased

/** @} */

/**
 * @brief Função da tarefa que executa o método.
 *
 * Esta função é responsável por executar o método.
 *
 * @param pvParameters Parâmetros da tarefa.
 */
void TaskRunMetodo(void *pvParameters);

/**
 * @brief Função da tarefa que lê dados da serial.
 *
 * Esta função é responsável por ler os dados recebidos da serial.
 *
 * @param pvParameters Parâmetros da tarefa.
 */
void TaskReadSerial(void *pvParameters);

/**
 * @brief Função da tarefa que controla o LED.
 *
 * Esta função é responsável por controlar o LED como atuador.
 *
 * @param pvParameters Parâmetros da tarefa.
 */
void TaskBlink(void *pvParameters);

/**
 * @brief Função da tarefa que pega comandos para iniciar o método.
 *
 * Esta função é responsável por pegar comandos para iniciar o método.
 *
 * @param pvParameters Parâmetros da tarefa.
 */
void TaskCmd(void *pvParameters);

/**
 * @brief Função da tarefa que realiza a saída dos dados.
 *
 * Esta função é responsável por realizar a saída dos dados.
 *
 * @param pvParameters Parâmetros da tarefa.
 */
void TaskOutput(void *pvParameters);


#endif