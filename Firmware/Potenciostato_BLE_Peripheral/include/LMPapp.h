/**
 * @file LMPapp.h
 * @brief Definições e estruturas para o aplicativo LMP.
 */

#ifndef LMPAPP_H
#define LMPAPP_H

#include <Arduino.h>
#include "LMP91000.h"
#include "dac80501.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ads131a04.h"
#include "dac80501.h"


/**
 * @brief Struct de configuração inicial do LMP.
 */
struct config_lmp {
    uint8_t lmpGain;
    uint8_t RLoad;
    uint8_t IntZ;
    uint8_t bias_setting;
};

/**
 * @brief Struct de configuração de Cyclic Voltammetry (CV).
 */
struct CV_setup {
    int lmpGain;
    int cycles;
    int startV;
    int endV;
    int vertex1;
    int vertex2;
    int stepV;
    int rate;
    int setToZero;
};

/**
 * @brief Struct de configuração de Amperometria.
 */
struct Amp_setup {
    int lmpGain;
    int pre_stepV;
    int quietTime;
    int v1;
    int t1;
    int v2;
    int t2;
    int samples;
    int range;
    int setToZero;
};


/**
 * @brief Union de configuração de métodos.
 */
typedef union {
    CV_setup cv;       ///< Configuração para CV 
    Amp_setup amp;     ///< Configuração para Amperometria
} UnionMetodos;

/**
 * @brief Enumeração para tipos de métodos.
 */
typedef enum {
    CV_metodo = 0,    ///< Cyclic Voltammetry (CV)
    Amp_metodo = 1,   ///< Amperometria
    Generic_type = 9  ///< Tipo Genérico
} MetodoType;

/**
 * @brief Struct para controle de método.
 */
typedef struct {
    MetodoType type;           ///< Tipo de método
    UnionMetodos metodo;       ///< Configuração do método
} StructControlMetodo;

/**
 * @brief Struct de dados para impressão em CV.
 */
struct data_print_CV {
  uint16_t zero;
  double bias;
  float vout;
  float c1;
  float current;
  float current2;
  uint8_t cycle;
  float voltage_f;
  float current_f;
};

/**
 * @brief Struct de dados para impressão em Amperometria.
 */
struct data_print_Amp {
  float voltage;
  unsigned long time;
  float current;
};

/**
 * @brief Union de dados de impressão.
 */
typedef union {
    data_print_CV cv;        ///< Dados de impressão para CV
    data_print_Amp amp;      ///< Dados de impressão para Amperometria
    char string[100];
} UnionOutput;

/**
 * @brief Struct para controle de saída.
 */
typedef struct {
    MetodoType type;           ///< Tipo de método
    UnionOutput output;        ///< Dados de saída
} StructControlOutput;


/**
 * @brief Inicializa o LMP com os pinos MENB.
 *
 * @param MENB Pino MENB (Enable) do LMP.
 */
void setupLMP(uint8_t MENB);

/**
 * @brief Inicializa o aplicativo LMP com configurações e ponteiro para DAC.
 *
 * @param config Configuração do LMP.
 * @param dac Ponteiro para o DAC.
 */
void initLMPapp(config_lmp, dac_t*);

/**
 * @brief Inicializa o LMP com o pino MENB.
 *
 * @param MENB Pino MENB (Enable) do LMP.
 */
void initLMP(uint8_t);

/**
 * @brief Verifica se o LMP está pronto.
 *
 * @return `true` se o LMP estiver pronto, `false` caso contrário.
 */
boolean LMP_isReady(void);


/**
 * @brief Tarefa para executar o método.
 *
 * @param pvParameters Parâmetros da tarefa.
 */
void TaskRunMetodo(void *pvParameters);

/**
 * @brief Define a tensão do LMP.
 *
 * @param voltage Tensão desejada.
 */
void LMP_setVoltage(int16_t voltage);

/**
 * @brief Lê a tensão do LMP.
 *
 * @param voltage Tensão lida.
 */
void LMP_readVoltage(int16_t voltage);

/**
 * @brief Define o bias do LMP.
 *
 * @param voltage Bias desejado.
 */
inline void setLMPBias(int16_t voltage);

/**
 * @brief Define a tensão.
 *
 * @param voltage Tensão desejada.
 */
inline void setVoltage(int16_t voltage);

/**
 * @brief Imprime os dados de CV.
 */
void print_data_CV();

/**
 * @brief Define as saídas para zero.
 */
void setOutputsToZero();

/**
 * @brief Inicia o método.
 */
void iniciarMetodo();

/**
 * @brief Para o método.
 */
void pararMetodo();

/**
 * @brief Reinicia o método.
 */
void reiniciarMetodo();

#endif