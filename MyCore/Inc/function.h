#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "main.h"

#define CCMRAM __attribute__((section("ccmram")))

#define ADC_MAX_VALUE 8190.0F
#define REF_3V3 3.3006F
#define TS_CAL1 *((__IO uint16_t *)0x1FFF75A8)
#define TS_CAL2 *((__IO uint16_t *)0x1FFF75CA)
#define TS_CAL1_TEMP 30.0F
#define TS_CAL2_TEMP 130.0F

#define MAX_SHORT_I 10.1F
#define MIN_SHORT_V 0.5F

#define MIN_BUKC_DUTY 100
#define MAX_BUCK_DUTY 28200
#define MAX_BUCK_DUTY1 24000
#define MIN_BOOST_DUTY 100
#define MIN_BOOST_DUTY1 1800
#define MAX_BOOST_DUTY 19500
#define MAX_BOOST_DUTY1 28200

#define CAL_VOUT_K 4099
#define CAL_VOUT_B 1
#define CAL_IOUT_K 4095
#define CAL_IOUT_B 1

#define F_NOERR 0x0000
#define F_SW_VIN_UVP 0x0001
#define F_SW_VIN_OVP 0x0002
#define F_SW_VOUT_UVP 0x0004
#define F_SW_VOUT_OVP 0x0008
#define F_SW_IOUT_OCP 0x0010
#define F_SW_SHORT 0x0020
#define F_OTP 0x0040

struct _SET_Value
{
	volatile float SET_modified_flag;
	volatile float Vout;
	volatile float Iout;
	volatile uint8_t currentSetting;
	volatile uint8_t SET_bit;
};

struct _Ctr_value
{
	volatile int32_t Vout_ref;
	volatile int32_t Vout_SSref;
	volatile int32_t Vout_SETref;
	volatile int32_t Iout_ref;
	volatile int32_t I_Limit;
	volatile int16_t BUCKMaxDuty;
	volatile int16_t BoostMaxDuty;
	volatile int16_t BuckDuty;
	volatile int16_t BoostDuty;
	volatile int32_t Ilimitout;
};

struct _FLAG
{
	volatile uint16_t SMFlag;
	volatile uint16_t CtrFlag;
	volatile uint16_t ErrFlag;
	volatile uint8_t BBFlag;
	volatile uint8_t PWMENFlag;
	volatile uint8_t BBModeChange;
	volatile uint8_t OUTPUT_Flag;
};

struct _ADI
{
	volatile uint32_t Iout;
	volatile uint32_t IoutAvg;
	volatile uint32_t Vout;
	volatile uint32_t VoutAvg;
	volatile uint32_t Iin;
	volatile uint32_t IinAvg;
	volatile uint32_t Vin;
	volatile uint32_t VinAvg;
};

typedef enum
{
	Init,
	Wait,
	Rise,
	Run,
	Err
} STATE_M;

typedef enum
{
	NA,
	Buck,
	Boost,
	Mix
} BB_M;

typedef enum
{
	SSInit,
	SSWait,
	SSRun
} SState_M;

typedef enum
{
	VIset_page = 1,
	DATA1_page,
	DATA2_page,
	SET_page
} _Screen_page;

typedef enum
{
	CV,
	CC
} _CVCC_Mode;

extern volatile uint16_t ADC1_RESULT[4];
extern volatile uint8_t BUZZER_Short_Flag;
extern volatile uint8_t BUZZER_Flag;
extern volatile uint8_t BUZZER_Middle_Flag;
extern struct _Ctr_value CtrValue;
extern struct _FLAG DF;
extern struct _ADI SADC;
extern volatile float VIN, VOUT, IIN, IOUT;
extern volatile float MainBoard_TEMP, CPU_TEMP;
extern volatile float powerEfficiency;
extern volatile _CVCC_Mode CVCC_Mode;
extern struct _SET_Value SET_Value;

extern volatile float MAX_OTP_VAL;
extern volatile float MAX_VOUT_OVP_VAL;
extern volatile float MAX_VOUT_OCP_VAL;
extern volatile uint8_t Encoder_Flag;
extern SState_M STState;
extern _Screen_page Screen_page;

void Init_Flash(void);
void Update_Flash(void);
void Read_Flash(void);
void float_to_bytes(float value, uint8_t *bytes);
float bytes_to_float(uint8_t *bytes);

void Key_Process(void);
void Encoder(void);
void ADCSample(void);
void ADC_calculate(void);
void StateM(void);
void StateMInit(void);
void ValInit(void);
void StateMWait(void);
void StateMRise(void);
void StateMRun(void);
void StateMErr(void);
void ShortOff(void);
void OVP(void);
void OCP(void);
void OTP(void);
void BBMode(void);
void BUZZER_Short(void);
void BUZZER_Middle(void);
float GET_NTC_Temperature(void);
float GET_CPU_Temperature(void);
float calculateTemperature(float voltage);
float one_order_lowpass_filter(float input, float alpha);
void FAN_PWM_set(uint16_t dutyCycle);
void Auto_FAN(void);
void OLED_Display(void);

#define setRegBits(reg, mask) (reg |= (unsigned int)(mask))
#define clrRegBits(reg, mask) (reg &= (unsigned int)(~(unsigned int)(mask)))
#define getRegBits(reg, mask) (reg & (unsigned int)(mask))
#define getReg(reg) (reg)

#endif
