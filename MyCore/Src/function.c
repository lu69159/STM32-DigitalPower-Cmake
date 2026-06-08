#include "function.h"

volatile uint16_t ADC1_RESULT[4] = {0, 0, 0, 0};
struct _Ctr_value CtrValue = {0};
struct _FLAG DF = {0, 0, 0, 0, 0, 0, 0};
struct _ADI SADC = {0};
volatile float VIN = 0, VOUT = 0, IIN = 0, IOUT = 0;
volatile float MainBoard_TEMP = 0, CPU_TEMP = 0;
volatile float powerEfficiency = 0;
struct _SET_Value SET_Value = {0};
volatile uint8_t BUZZER_Short_Flag = 0;
volatile uint8_t BUZZER_Flag = 0;
volatile uint8_t BUZZER_Middle_Flag = 0;
