#include "task.h"
#include "main.h"
#include "stm32g4xx_hal.h"
#include "tim.h"
#include "adc.h"
#include "hrtim.h"
#include "oled.h"
#include "function.h"
#include "Key.h"
#include "PID.h"
#include "iwdg.h"
#include "usart.h"
#include "gpio.h"

extern volatile uint16_t ms_cnt_1;
extern volatile uint16_t ms_cnt_2;
extern volatile uint16_t ms_cnt_3;
extern volatile uint16_t ms_cnt_4;

void Task_Init(void){
  DF.SMFlag = Init;                         // 初始化状态机
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3); // 启动定时器8和通道3的PWM输出
  FAN_PWM_set(100);                         // 设置风扇转速为100%
  HAL_Delay(20);
  OLED_Init();
  OLED_NewFrame();
  OLED_PrintString(0, 0, "Loading...", &font16x16, OLED_COLOR_NORMAL);
  OLED_ShowFrame();
  HAL_TIM_Base_Start_IT(&htim2);            // 1kHz
  HAL_TIM_Base_Start_IT(&htim3);            // 200Hz
  HAL_TIM_Base_Start_IT(&htim4);            // 100Hz
  Key_Init();
  PID_Init();
  Init_Flash();
  Read_Flash();                             // 读取Flash数据

  HAL_Delay(200);                                        // 延时200ms，等待供电稳定
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); // 校准ADC1
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED); // 校准ADC2
  HAL_ADCEx_Calibration_Start(&hadc5, ADC_SINGLE_ENDED); // 校准ADC5
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC1_RESULT, 4); // 启动ADC1采样和DMA数据传送,采样输入输出电压电流
  HAL_ADC_Start(&hadc2);                                 // 启动ADC2采样，采样NTC温度
  HAL_ADC_Start(&hadc5);                                 // 启动ADC5采样，采样单片机CPU温度

  HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_D);
  HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_F);      // 开启HRTIM波形计数器
  __HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_TIM_IT_REP); // 开启HRTIM定时器D的中断
  HAL_GPIO_WritePin(GPIOC, LED_G_Pin | LED_R_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET); // 关闭LED_G和LED_R，关闭蜂鸣器
  FAN_PWM_set(0); // 设置风扇转速为0
}

void Task_Run(void){    
    Encoder();

    if (ms_cnt_3 >= 10)
    {
      ms_cnt_3 = 0;
      BUZZER_Short();
      ADC_calculate();
    }

    if (ms_cnt_4 >= 50)
    {
      ms_cnt_4 = 0;
      BUZZER_Middle();

      if ((DF.SMFlag == Rise) || (DF.SMFlag == Run))
      {
        HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
      }
      else
      {
        HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
      }

      if (IOUT >= 0.1)
      {
        powerEfficiency = (VOUT * IOUT) / (VIN * IIN) * 100.0;
      }
      else
      {
        powerEfficiency = 0;
      }

      USART1_Printf("%.3f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%d\n", VIN, IIN, VOUT, IOUT, MainBoard_TEMP, CPU_TEMP, powerEfficiency, CVCC_Mode);
    }

    if (ms_cnt_2 >= 100)
    {
      ms_cnt_2 = 0;
      OLED_Display();
      Auto_FAN();
    }

    if (ms_cnt_1 >= 500)
    {
      ms_cnt_1 = 0;
      HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
      Update_Flash();
    }

    HAL_IWDG_Refresh(&hiwdg);
    //HRTIM1中断还有很大问题，以及OLED的数值显示
}

void test(char *str){
  OLED_NewFrame();
  OLED_PrintString(0, 0, str, &font16x16, OLED_COLOR_NORMAL);
  OLED_ShowFrame();
}
