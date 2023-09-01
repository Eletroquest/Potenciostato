/**
 * @file LMPapp.cpp
 * @brief Implementação do aplicativo LMP.
 */

#include "LMPapp.h"
#include "ads131a04.h"
#include "dac80501.h"

// Variáveis externas
extern bool inicia_blink;
extern xQueueHandle CVsetup_queue;
extern xQueueHandle Ampsetup_queue;
extern xQueueHandle Metodo_queue;
extern TaskHandle_t taskMetodo_Handle;
extern xQueueHandle output_queue;
extern SemaphoreHandle_t xSemaphore_writeLMP;

//Inicializa variaveis
LMP91000 pStat = LMP91000();        //Instância do objeto LMP91000.
adc_data_struct dataStruct;         //Estrutura de dados para os resultados da conversão A/D.
data_print_CV data_CV_f;            //Estrutura de dados para imprimir resultados de CV.
data_print_Amp data_Amp_f;          //Estrutura de dados para imprimir resultados de Amperometria.
dac_t dac_f;                        //Estrutura de dados para controle do DAC.
uint8_t bias_setting = 0;           //Configuração de ajuste de polarização.
uint16_t dacVout = 1500;            //Tensão de saída do DAC.
const float v_tolerance = 0.008;    //Tolerância para a tensão de saída do DAC.
const uint16_t opVolt = 4900;       //Tensão operacional do DAC.
uint8_t LMPgain = 0;                //Ganho do LMP.
uint8_t IntZ = 2;                   //Impedância interna do LMP.
float vref = 5;                     //Tensão de referência.
float RFB = 10000;                  //Resistor de realimentação (feedback resistor).

/**
 * @brief Tarefa para executar o método.
 *
 * @param pvParameters Parâmetros da tarefa.
 */
void TaskRunMetodo(void *pvParameters)
{
  std::string current = "";
  int16_t j;
  StructControlMetodo metodo_r;
  StructControlOutput send_data;

  for (;;)
  {
    if (xQueueReceive(Metodo_queue, &metodo_r, 500 / portTICK_PERIOD_MS))
    {
      if (metodo_r.type == CV_metodo)
      { // inicia CV
        if (metodo_r.metodo.cv.vertex1 > metodo_r.metodo.cv.startV)
        {
          initLMP(metodo_r.metodo.cv.lmpGain);
          metodo_r.metodo.cv.stepV = abs(metodo_r.metodo.cv.stepV);
          metodo_r.metodo.cv.rate = (1000.0 * metodo_r.metodo.cv.stepV) / metodo_r.metodo.cv.rate;
          send_data.type = Generic_type;
          strcpy(send_data.output.string, "Cycle\tVoltage\tCurrent \n");
          xQueueSend(output_queue, &send_data, 0);
          // Serial.println(F("Zero\tBIAS\tVout\tC1\tCurrent\tCycle\tVoltage\tCurrent"));
          j = metodo_r.metodo.cv.startV;

          for (uint8_t i = 0; i < metodo_r.metodo.cv.cycles; i++)
          {
            data_CV_f.cycle = i + 1;
            for (j; j <= metodo_r.metodo.cv.vertex1; j += metodo_r.metodo.cv.stepV)
            {
              LMP_setVoltage(j);
              vTaskDelay(metodo_r.metodo.cv.rate / portTICK_PERIOD_MS);
              LMP_readVoltage(j);
              send_data.output.cv = data_CV_f;
              send_data.type = metodo_r.type;
              xQueueSend(output_queue, &send_data, 0);
              // print_data_CV();
            }
            j -= 2 * metodo_r.metodo.cv.stepV; // incrementa j duas vezes para evitar polarização no vértice duas vezes

            // j começa logo abaixo do primeiro vértice
            for (j; j >= metodo_r.metodo.cv.vertex2; j -= metodo_r.metodo.cv.stepV)
            {
              LMP_setVoltage(j);
              vTaskDelay(metodo_r.metodo.cv.rate / portTICK_PERIOD_MS);
              LMP_readVoltage(j);
              send_data.output.cv = data_CV_f;
              send_data.type = metodo_r.type;
              xQueueSend(output_queue, &send_data, 0);
              // print_data_CV();
            }
            j += 2 * metodo_r.metodo.cv.stepV; // incrementa j duas vezes para evitar polarização no vértice duas vezes

            // j começa logo acima do segundo vértice
            for (j; j <= metodo_r.metodo.cv.endV; j += metodo_r.metodo.cv.stepV)
            {
              LMP_setVoltage(j);
              vTaskDelay(metodo_r.metodo.cv.rate / portTICK_PERIOD_MS);
              LMP_readVoltage(j);
              send_data.output.cv = data_CV_f;
              send_data.type = metodo_r.type;
              xQueueSend(output_queue, &send_data, 0);
              // print_data_CV();
            }
          }
        }
        else
        {
          initLMP(metodo_r.metodo.cv.lmpGain);
          metodo_r.metodo.cv.stepV = abs(metodo_r.metodo.cv.stepV);
          metodo_r.metodo.cv.rate = (1000.0 * metodo_r.metodo.cv.stepV) / metodo_r.metodo.cv.rate;
          send_data.type = Generic_type;
          strcpy(send_data.output.string, "Cycle\tVoltage\tCurrent \n");
          xQueueSend(output_queue, &send_data, 0);
          // Serial.println(F("Zero\tBIAS\tVout\tC1\tCurrent\tCycle\tVoltage\tCurrent"));
          j = metodo_r.metodo.cv.startV;

          for (uint8_t i = 0; i < metodo_r.metodo.cv.cycles; i++)
          {
            data_CV_f.cycle = i + 1;
            for (j; j >= metodo_r.metodo.cv.vertex1; j -= metodo_r.metodo.cv.stepV)
            {
              LMP_setVoltage(j);
              vTaskDelay(metodo_r.metodo.cv.rate / portTICK_PERIOD_MS);
              LMP_readVoltage(j);
              send_data.output.cv = data_CV_f;
              send_data.type = metodo_r.type;
              xQueueSend(output_queue, &send_data, 0);
              // print_data_CV();
            }
            j += 2 * metodo_r.metodo.cv.stepV; // incrementa j duas vezes para evitar polarização no vértice duas vezes

            // j começa logo abaixo do primeiro vértice
            for (j; j <= metodo_r.metodo.cv.vertex2; j += metodo_r.metodo.cv.stepV)
            {
              LMP_setVoltage(j);
              vTaskDelay(metodo_r.metodo.cv.rate / portTICK_PERIOD_MS);
              LMP_readVoltage(j);
              send_data.output.cv = data_CV_f;
              send_data.type = metodo_r.type;
              xQueueSend(output_queue, &send_data, 0);
              // print_data_CV();
            }
            j -= 2 * metodo_r.metodo.cv.stepV; // incrementa j duas vezes para evitar polarização no vértice duas vezes

            // j começa logo acima do segundo vértice
            for (j; j >= metodo_r.metodo.cv.endV; j -= metodo_r.metodo.cv.stepV)
            {
              LMP_setVoltage(j);
              vTaskDelay(metodo_r.metodo.cv.rate / portTICK_PERIOD_MS);
              LMP_readVoltage(j);
              send_data.output.cv = data_CV_f;
              send_data.type = metodo_r.type;
              xQueueSend(output_queue, &send_data, 0);
              // print_data_CV();
            }
          }
        }
        send_data.type = Generic_type;
        strcpy(send_data.output.string, "end_CV\n");
        xQueueSend(output_queue, &send_data, 0);
      }
      else if (metodo_r.type == Amp_metodo)
      {
        if (metodo_r.metodo.amp.range == 12)
          current = "Current(pA)";
        else if (metodo_r.metodo.amp.range == 9)
          current = "Current(nA)";
        else if (metodo_r.metodo.amp.range == 6)
          current = "Current(uA)";
        else if (metodo_r.metodo.amp.range == 3)
          current = "Current(mA)";
        else
          current = "SOME ERROR";

        initLMP(metodo_r.metodo.amp.lmpGain);

        int16_t voltageArray[3] = {metodo_r.metodo.amp.pre_stepV, metodo_r.metodo.amp.v1, metodo_r.metodo.amp.v2};
        uint32_t timeArray[3] = {metodo_r.metodo.amp.quietTime, metodo_r.metodo.amp.t1, metodo_r.metodo.amp.t2};

        // Envia cabeçalho do metodo
        send_data.type = Generic_type;
        std::string saida_cp = "Voltage(mV)\tTime(ms)\t" + current + "\n";
        strcpy(send_data.output.string, saida_cp.c_str());
        xQueueSend(output_queue, &send_data, 0);

        for (uint8_t i = 0; i < 3; i++)
        {
          // o tempo entre as amostras é determinado pelo
          // número de amostras inserido pelo usuário
          uint32_t fs = (double)timeArray[i] / metodo_r.metodo.amp.samples;
          unsigned long startTime = millis();

          LMP_setVoltage(voltageArray[i]);

          while (millis() - startTime < timeArray[i])
          {
            readData(&dataStruct);

            float v1 = 2 * vref * (float(dataStruct.channel1) / (pow(2, 24)));
            float v2 = 2 * vref * (float(dataStruct.channel2) / (pow(2, 24)));
            float current = 0;

            if (LMPgain == 0)
              current = (((v1 - v2) / 1000) / RFB) * pow(10, metodo_r.metodo.amp.range); // scales to nA
            else
              current = (((v1 - v2) / 1000) / TIA_GAIN[LMPgain - 1]) * pow(10, metodo_r.metodo.amp.range); // scales to nA

            float current2 = ((((v2 - v1)) / (TIA_GAIN[LMPgain - 1])));

            // seta para imprimir os dados
            data_Amp_f.voltage = voltageArray[i];
            data_Amp_f.time = millis();
            data_Amp_f.current = current;
            send_data.output.amp = data_Amp_f;
            send_data.type = metodo_r.type;
            xQueueSend(output_queue, &send_data, 0);

            vTaskDelay((fs - 1) / portTICK_PERIOD_MS);
          }
        }

        send_data.type = Generic_type;
        strcpy(send_data.output.string, "end_Amp\n");
        xQueueSend(output_queue, &send_data, 0);
      }
      vTaskDelay(200 / portTICK_PERIOD_MS);
      inicia_blink = false;
    }
  }
}

/**
 * @brief Inicializa o LMP com o pino MENB.
 *
 * @param MENB Pino MENB (Enable) do LMP.
 */
void setupLMP(uint8_t MENB)
{
  pStat.setMENB(MENB);
  pStat.standby();
}

/**
 * @brief Define a tensão do LMP.
 *
 * @param voltage Tensão desejada.
 */
void LMP_setVoltage(int16_t voltage)
{
  setLMPBias(voltage);
  setVoltage(voltage);
}

/**
 * @brief Lê a tensão do LMP.
 *
 * @param voltage Tensão lida.
 */
void LMP_readVoltage(int16_t voltage)
{
  // tensão de saída do amplificador transimpedância
  readData(&dataStruct);

  float v1 = 0;
  if (IntZ == 3)
    v1 = (float(dacVout) / 1000);
  else
    v1 = (float(dacVout) / 1000) * TIA_ZERO[IntZ];
  float v2 = 2 * vref * (float(dataStruct.channel2) / (pow(2, 24)));
  float current = 0;
  // a corrente é determinada pelo zero do amplificador transimpedância
  // a partir da saída do amplificador transimpedância, e depois dividindo
  // pelo resistor de feedback
  // corrente = (V_OUT - V_IN) / RFB
  // v1 e v2 estão em milivolts
  if (LMPgain == 0)
    current = (((v2 - v1) / 1000) / RFB) * pow(10, 9); // escala para nA
  else
    current = (((v2 - v1) / 1000) / TIA_GAIN[LMPgain - 1]) * pow(10, 9); // escalado para uA pow(10,6)=uA ou pow(10,9)=nA

  float current2 = ((((v2 - v1)) / (TIA_GAIN[LMPgain - 1])));
  
  data_CV_f.vout = v2;
  data_CV_f.c1 = v1;
  data_CV_f.current = current;
  data_CV_f.current2 = current2 * pow(10, 6);
  data_CV_f.voltage_f = voltage;
  data_CV_f.current_f = current;
}


/**
 * @brief Define a tensão do DAC.
 *
 * @param voltage Tensão desejada.
 */
void setVoltage(int16_t voltage)
{
  // Tensão DAC mínima que pode ser definida
  // O LMP91000 aceita um valor mínimo de 1,5V, adicionando os
  // 20 mV adicionais para ter uma margem de segurança
  const uint16_t minDACVoltage = 1520;
  bias_setting = 0;

  if (voltage == 0)
  {
    dacVout = 1500;
  }
  else
  {
    dacVout = minDACVoltage;
    // a tensão não pode ser ajustada para menos de 15mV, porque o LMP91000
    // aceita um mínimo de 1,5V no pino VREF e possui 1% como sua
    // menor opção de polarização 1,5V * 1% = 15mV
    if (abs(voltage) < 15)
      voltage = 15 * (voltage / abs(voltage));

    int16_t setV = dacVout * TIA_BIAS[bias_setting];
    voltage = abs(voltage);

    while (setV > voltage * (1 + v_tolerance) || setV < voltage * (1 - v_tolerance))
    {
      if (bias_setting == 0)
        bias_setting = 1;

      dacVout = voltage / TIA_BIAS[bias_setting];

      if (dacVout > opVolt)
      {
        bias_setting++;
        dacVout = 1500;
        if (bias_setting > NUM_TIA_BIAS)
        {
          bias_setting = 0;
        }
      }

      setV = dacVout * TIA_BIAS[bias_setting];
    }
  }
  xSemaphoreTake(xSemaphore_writeLMP, portMAX_DELAY);
  pStat.setBias(bias_setting);
  dac_set_vout(&dac_f, dacVout);
  xSemaphoreGive(xSemaphore_writeLMP);

  data_CV_f.zero = dacVout;
  data_CV_f.bias = TIA_BIAS[bias_setting];
}


/**
 * @brief Define o bias do LMP.
 *
 * @param voltage Bias desejado.
 */
inline void setLMPBias(int16_t voltage)
{
  signed char sign = (float)voltage / abs(voltage);

  if (sign < 0)
    pStat.setNegBias();
  else if (sign > 0)
    pStat.setPosBias();
  else
  {   // não faz nada
  } 
}

/**
 * @brief Inicializa o aplicativo LMP com configurações e ponteiro para DAC.
 *
 * @param config Configuração do LMP.
 * @param dac Ponteiro para o DAC.
 */
void initLMPapp(config_lmp config_lmp_d, dac_t *dac_copy)
{
  dac_f = *dac_copy;

  pStat.disableFET();
  pStat.setGain(config_lmp_d.lmpGain);
  LMPgain = config_lmp_d.lmpGain;
  pStat.setRLoad(config_lmp_d.RLoad);
  pStat.setExtRefSource();
  pStat.setIntZ(config_lmp_d.IntZ);
  IntZ = config_lmp_d.IntZ;
  pStat.setThreeLead();
  pStat.setBias(config_lmp_d.bias_setting);
  pStat.setPosBias();
  pStat.setBias(0);
}


/**
 * @brief Inicializa o LMP com o ganho especificado.
 *
 * @param lmpGain Ganho do LMP.
 */
void initLMP(uint8_t lmpGain)
{
  pStat.setGain(lmpGain);
  pStat.setBias(0);
  LMPgain = lmpGain;
}

/**
 * @brief Verifica se o LMP está pronto.
 *
 * @return `true` se o LMP estiver pronto, `false` caso contrário.
 */
boolean LMP_isReady()
{
  return pStat.isReady();
}

/**
 * @brief Imprime os dados de CV.
 */
void print_data_CV()
{
  Serial.print(data_CV_f.zero);
  Serial.print(F("\t"));
  Serial.print(data_CV_f.bias);
  Serial.print(F("\t"));
  Serial.print(data_CV_f.vout, 10);
  Serial.print(F("\t"));
  Serial.print(data_CV_f.c1, 10);
  Serial.print(F("\t"));
  Serial.print(data_CV_f.current, 10);
  Serial.print(F("\t"));
  Serial.print(data_CV_f.cycle);
  Serial.print(F("\t"));
  Serial.print(data_CV_f.voltage_f);
  Serial.print(F("\t"));
  Serial.print(data_CV_f.current2, 4);
  Serial.print(F("\t"));
  Serial.println();
}

/**
 * @brief Define as saídas para zero.
 */
void setOutputsToZero()
{
  dac_set_vout(&dac_f, 0);
}

/**
 * @brief Inicia a execução do método.
 */
void iniciarMetodo()
{
  // Cria a tarefa
  BaseType_t taskCreationResult = xTaskCreate(
      TaskRunMetodo, "Metodo" // A name just for humans
      ,
      8192 // tamanho da task
      ,
      NULL, 1 // Prioridade, sendo 1 a mais alta e 4 a mais baixa.
      ,
      &taskMetodo_Handle); 
}

/**
 * @brief Para a execução do método.
 */
void pararMetodo()
{
  // Exclui a tarefa atual
  vTaskDelete(taskMetodo_Handle);
}

/**
 * @brief Reinicia a execução do método.
 */
void reiniciarMetodo()
{
  // Para a tarefa atual
  pararMetodo();

  // Cria uma nova tarefa
  iniciarMetodo();
}