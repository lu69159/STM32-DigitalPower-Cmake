/*
 * task.c
 *
 *  Created on: Jun 25, 2026
 *      Author: Administrator1
 */
#include "main.h"
#include "task.h"
#include "hrtim.h"
#include "oled.h"
#include <stdio.h>
#include <math.h>

#define TS_CAL1 *((__IO uint16_t *)0x1FFF75A8) // 内部温度传感器在30度和VREF为3V时的校准数据
#define TS_CAL2 *((__IO uint16_t *)0x1FFF75CA) // 内部温度传感器在130度和VREF为3V时的校准数据

volatile uint8_t page = 1;
volatile float compare = 0;

volatile float VIN, VOUT, IIN, IOUT;
volatile float TMAIN, TMCU;
volatile uint16_t ADCresult[4] = {0, 0, 0, 0};

float VSET = 5.0, ISET = 0.5;

int count = 0; //旋转编码器计数
volatile int timer = 0, timer2 = 0; //时间计数

enum pageState state = pageMenu;
Key key1 = {KEY1_GPIO_Port, KEY1_Pin, keyIdle, 0};
Key	key2 = {KEY2_GPIO_Port, KEY2_Pin, keyIdle, 0};
Key	keyEncoder = {Encoder_KEY_GPIO_Port, Encoder_KEY_Pin, keyIdle, 0};

extern TIM_HandleTypeDef htim1, htim2, htim3, htim4;
extern ADC_HandleTypeDef hadc1, hadc2, hadc5;
extern IWDG_HandleTypeDef hiwdg;

extern HAL_StatusTypeDef HAL_HRTIM_WaveformCompareConfig(HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx, uint32_t CompareUnit, const HRTIM_CompareCfgTypeDef *pCompareCfg);

void taskInit(){
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET); //蜂鸣器闭嘴！
	HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);

	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1 | TIM_CHANNEL_2); //旋转编码器
	__HAL_TIM_SET_COUNTER(&htim1, page); //初始页面
	//HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3); //风扇的PWM
	HAL_Delay(20);  //OLED延时
	OLED_Init();
	OLED_NewFrame();
	OLED_PrintString(0, 0, "Loading...", &font16x16, OLED_COLOR_NORMAL);
	OLED_ShowFrame();
	HAL_TIM_Base_Start_IT(&htim2);       // 100Hz，时间计数
	HAL_TIM_Base_Start_IT(&htim3);       // 200Hz，电源状态监测
	HAL_TIM_Base_Start_IT(&htim4);       // 100Hz，按键扫描

	HAL_Delay(200);	//等待供电稳定
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc5, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADCresult, 4); //采样输入输出电压电流
	HAL_ADC_Start(&hadc2);	//采样NTC温度
	HAL_ADC_Start(&hadc5);	//采样单片机CPU温度

	//BUCKBOOST电路控制信号 30000为100%
	HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_D);                     // 开启HRTIM波形计数器
	HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_F);                     // 开启HRTIM波形计数器
	__HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_TIM_IT_REP); // 开启HRTIM定时器D的中断

	HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
};

void taskRun(){
	//旋转编码器
	count = __HAL_TIM_GET_COUNTER(&htim1);
	if(state == pageMenu){
		if(count > 4){
			__HAL_TIM_SET_COUNTER(&htim1, 1);
			count = 1;
		}
		if(count < 1){
			__HAL_TIM_SET_COUNTER(&htim1, 4);
			count = 4;
		}

		page = count;
	}else if(state == pageChoose){
		if(count > 2){
			__HAL_TIM_SET_COUNTER(&htim1, 1);
			count = 1;
		}
		if(count < 1){
			__HAL_TIM_SET_COUNTER(&htim1, 2);
			count = 2;
		}
	}else{
		if(count > 10){
			__HAL_TIM_SET_COUNTER(&htim1, 1);
			count = 1;
		}
		if(count < 1){
			__HAL_TIM_SET_COUNTER(&htim1, 10);
			count = 10;
		}
	}

	if(timer > 10){
		float v = HAL_ADC_GetValue(&hadc2) * 3.3 / 65520.0, Rt = (3.3 - v) * 10000.0F / v;// NTC电阻
		TMAIN = 1.0F / (1.0F / (273.15F + 25) + log(Rt / 10000) / 3950) - 273.15F; // 10K固定阻值电阻下计算温度
		TMCU = (100/(TS_CAL2 - TS_CAL1))*((HAL_ADC_GetValue(&hadc5)/8)*1.1002 - TS_CAL1) + 30;
		timer2++;
		timer-=10;
	}
	if(timer2 > 10){
		HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin); //状态灯闪烁
		timer2 = 0;
	}

	//按键监听
	keyListening(&key1);
	keyListening(&key2);
	keyListening(&keyEncoder);

	//OLED
	oledPlay();

	//设置占空比
	//__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, 15000);

	HAL_IWDG_Refresh(&hiwdg);
};

//OLED
void oledPlay(){
	char tmp[50];

	OLED_NewFrame();
	switch(page){
		case 1:
			sprintf(tmp, "输入电压:%.2f", VIN);
			OLED_PrintString(0, 0, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 0, "V", &font16x16, OLED_COLOR_NORMAL);

			sprintf(tmp, "输入电流:%.2f", IIN);
			OLED_PrintString(0, 16, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 16, "A", &font16x16, OLED_COLOR_NORMAL);

			sprintf(tmp, "输出电压:%.2f", VOUT);
			OLED_PrintString(0, 32, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 32, "V", &font16x16, OLED_COLOR_NORMAL);

			sprintf(tmp, "输出电流:%.2f", IOUT);
			OLED_PrintString(0, 48, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 48, "A", &font16x16, OLED_COLOR_NORMAL);

			break;
		case 2:
			if(state == pageEdit){
				OLED_PrintString(0, 0, "设置电压:", &font16x16, OLED_COLOR_NORMAL);
				OLED_PrintString(0, 16, "设置电流:", &font16x16, OLED_COLOR_NORMAL);//72接下一个字
				//TODO
			}else{
				sprintf(tmp, "设置电压:%.2f", VSET);
				OLED_PrintString(0, 0, tmp, &font16x16, state == pageChoose ? count == 1 ? OLED_COLOR_REVERSED : OLED_COLOR_NORMAL : OLED_COLOR_NORMAL);

				sprintf(tmp, "设置电流:%.2f", ISET);
				OLED_PrintString(0, 16, tmp, &font16x16, state == pageChoose ? count == 2 ? OLED_COLOR_REVERSED : OLED_COLOR_NORMAL : OLED_COLOR_NORMAL);
			}

			OLED_PrintString(112, 0, "V", &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 16, "A", &font16x16, OLED_COLOR_NORMAL);

			sprintf(tmp, "输出电压:%.2f", VOUT);
			OLED_PrintString(0, 32, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 32, "V", &font16x16, OLED_COLOR_NORMAL);

			sprintf(tmp, "输出电流:%.2f", IOUT);
			OLED_PrintString(0, 48, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 48, "A", &font16x16, OLED_COLOR_NORMAL);

			break;
		case 3:
			sprintf(tmp, "过温保护:%.2f", 0);
			OLED_PrintString(0, 0, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 0, "C", &font16x16, OLED_COLOR_NORMAL);

			sprintf(tmp, "过流保护:%.2f", 0);
			OLED_PrintString(0, 16, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 16, "A", &font16x16, OLED_COLOR_NORMAL);

			sprintf(tmp, "过压保护:%.2f", 0);
			OLED_PrintString(0, 32, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 32, "V", &font16x16, OLED_COLOR_NORMAL);

			break;

		case 4:
			sprintf(tmp, "主板温度:%.2f", TMAIN);
			OLED_PrintString(0, 0, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 0, "C", &font16x16, OLED_COLOR_NORMAL);

			sprintf(tmp, "MCU温度:%.2f", TMCU);
			OLED_PrintString(0, 16, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 16, "C", &font16x16, OLED_COLOR_NORMAL);

			sprintf(tmp, "转换效率:%.1f", VOUT*IOUT/(VIN*IIN)*100.0);
			OLED_PrintString(0, 32, tmp, &font16x16, OLED_COLOR_NORMAL);
			OLED_PrintString(112, 32, "%", &font16x16, OLED_COLOR_NORMAL);

			break;
	}
	OLED_ShowFrame();
};

//KEY
void key(Key *key){
	if(HAL_GPIO_ReadPin(key->Port, key->Pin) == GPIO_PIN_RESET && !(key->state == keyPressed) && !key->isLocked){
		if(key->state == keyIdle){ //消抖
			key->state = keyShake;
		}else if(key->state == keyShake){
			key->state = keyPressed;
		}
	}else{
		if(key->isLocked){
			key->isLocked = 0;
			key->state = keyIdle;
		}
	}
};
void keyListening(Key *key){
	if(key->isLocked || key->state != keyPressed){
		return;
	}

	if(key->Port == key1.Port && key->Pin == key1.Pin){ //KEY1
		if(page == 2){
			if(state == pageMenu){
				__HAL_TIM_SET_COUNTER(&htim1, 1);
				count = 1;
				state = pageChoose;
			}else if(state == pageChoose){

			}
		}
	}else if(key->Port == key2.Port && key->Pin == key2.Pin){ //KEY2
		if(state == pageChoose){
			__HAL_TIM_SET_COUNTER(&htim1, page);
			count = page;
			state = pageMenu;
		}else if(state == pageEdit){

		}
	}else if(key->Port == keyEncoder.Port && key->Pin == keyEncoder.Pin){ //KEY ENCODER

	}
	key->isLocked = 1;
};

//ADC
void getADCresult(){
	static uint32_t Vin, Iin, Vout, Iout;

	Vin = (uint32_t)ADCresult[0];
	Iin = (uint32_t)ADCresult[1];
	Vout = (uint32_t)ADCresult[2];
	Iout = (uint32_t)ADCresult[3];

	VIN = Vin / 8190.0 * 3.3 / (4.7F / 75.0F);
	IIN = Iin / 8190.0 * 3.3 / 62.0F / 0.005F;
	VOUT = Vout / 8190.0 * 3.3 / (4.7F / 75.0F);
	IOUT = Iout / 8190.0 * 3.3  / 62.0F / 0.005F;
};



