#include "function.h"
#include "W25Q64.h"
#include "usart.h"
#include "tim.h"
#include "Key.h"
#include "hrtim.h"
#include "adc.h"
#include <string.h>
#include <math.h>

volatile uint16_t ADC1_RESULT[4] = {0, 0, 0, 0};
struct _Ctr_value CtrValue = {0, 0, 0, 0, 0, MIN_BUKC_DUTY, 0, 0, 0, 0};
struct _FLAG DF = {0, 0, 0, 0, 0, 0, 0};
struct _ADI SADC = {0};
volatile float VIN = 0, VOUT = 0, IIN = 0, IOUT = 0;
volatile float MainBoard_TEMP = 0, CPU_TEMP = 0;
volatile float powerEfficiency = 0;
volatile uint8_t Encoder_Flag = 0;
SState_M STState = SSInit;
_Screen_page Screen_page = VIset_page;
struct _SET_Value SET_Value = {0};
volatile uint8_t BUZZER_Short_Flag = 0;
volatile uint8_t BUZZER_Flag = 0;
volatile uint8_t BUZZER_Middle_Flag = 0;
volatile float MAX_OTP_VAL = 80.0F;
volatile float MAX_VOUT_OVP_VAL = 50.0F;
volatile float MAX_VOUT_OCP_VAL = 10.5F;

extern volatile int32_t VErr0, VErr1, VErr2;
extern volatile int32_t u0, u1;

void float_to_bytes(float value, uint8_t *bytes)
{
    memcpy(bytes, &value, sizeof(float));
}

float bytes_to_float(uint8_t *bytes)
{
    float value;
    memcpy(&value, bytes, sizeof(float));
    return value;
}

void Init_Flash(void)
{
    uint8_t Flash_flag[1];
    W25Q64_ReadData(0x000000, Flash_flag, 1);
    if (Flash_flag[0] != 0x00)
    {
        W25Q64_SectorErase(0x000000);
        uint8_t Flash_data[21];
        uint8_t VSETtemp[4], ISETtemp[4], OTPtemp[4], OCPtemp[4], OVPtemp[4];
        Flash_data[0] = 0x00;
        float_to_bytes(SET_Value.Vout, VSETtemp);
        float_to_bytes(SET_Value.Iout, ISETtemp);
        float_to_bytes(MAX_OTP_VAL, OTPtemp);
        float_to_bytes(MAX_VOUT_OCP_VAL, OCPtemp);
        float_to_bytes(MAX_VOUT_OVP_VAL, OVPtemp);
        for (uint8_t i = 0; i < 4; i++)
        {
            Flash_data[i + 1] = VSETtemp[i];
            Flash_data[i + 5] = ISETtemp[i];
            Flash_data[i + 9] = OTPtemp[i];
            Flash_data[i + 13] = OCPtemp[i];
            Flash_data[i + 17] = OVPtemp[i];
        }
        W25Q64_PageProgram(0x000000, Flash_data, 21);
    }
}

void Update_Flash(void)
{
    if (SET_Value.SET_modified_flag == 1)
    {
        W25Q64_SectorErase(0x000000);
        uint8_t Flash_data[21];
        uint8_t VSETtemp[4], ISETtemp[4], OTPtemp[4], OCPtemp[4], OVPtemp[4];
        Flash_data[0] = 0x00;
        float_to_bytes(SET_Value.Vout, VSETtemp);
        float_to_bytes(SET_Value.Iout, ISETtemp);
        float_to_bytes(MAX_OTP_VAL, OTPtemp);
        float_to_bytes(MAX_VOUT_OCP_VAL, OCPtemp);
        float_to_bytes(MAX_VOUT_OVP_VAL, OVPtemp);
        for (uint8_t i = 0; i < 4; i++)
        {
            Flash_data[i + 1] = VSETtemp[i];
            Flash_data[i + 5] = ISETtemp[i];
            Flash_data[i + 9] = OTPtemp[i];
            Flash_data[i + 13] = OCPtemp[i];
            Flash_data[i + 17] = OVPtemp[i];
        }
        W25Q64_PageProgram(0x000000, Flash_data, 21);
        SET_Value.SET_modified_flag = 0;
    }
}

void Read_Flash(void)
{
    uint8_t Flash_data[20];
    W25Q64_ReadData(0x000001, Flash_data, 20);
    uint8_t VSETtemp[4], ISETtemp[4], OTPtemp[4], OCPtemp[4], OVPtemp[4];
    for (uint8_t i = 0; i < 4; i++)
    {
        VSETtemp[i] = Flash_data[i];
        ISETtemp[i] = Flash_data[i + 4];
        OTPtemp[i] = Flash_data[i + 8];
        OCPtemp[i] = Flash_data[i + 12];
        OVPtemp[i] = Flash_data[i + 16];
    }
    SET_Value.Vout = bytes_to_float(VSETtemp);
    SET_Value.Iout = bytes_to_float(ISETtemp);
    MAX_OTP_VAL = bytes_to_float(OTPtemp);
    MAX_VOUT_OCP_VAL = bytes_to_float(OCPtemp);
    MAX_VOUT_OVP_VAL = bytes_to_float(OVPtemp);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == Encoder_A_Pin)
    {
        Encoder_Flag = 1;
    }
}

void Key_Process(void)
{
    if (Key_Flag[1] == 1)
    {
        BUZZER_Middle_Flag = 1;
        if (DF.SMFlag == Err)
        {
            DF.ErrFlag = F_NOERR;
        }
        else if (Screen_page == VIset_page)
        {
            if (SET_Value.SET_bit == 0)
            {
                SET_Value.currentSetting++;
                if (SET_Value.currentSetting > 2)
                {
                    SET_Value.currentSetting = 0;
                }
            }
        }
        else if (Screen_page == SET_page)
        {
            if (SET_Value.SET_bit == 0)
            {
                SET_Value.currentSetting++;
                if (SET_Value.currentSetting > 3)
                {
                    SET_Value.currentSetting = 0;
                }
            }
        }
        USART1_Printf("KEY1 Pressed\r\n");
        Key_Flag[1] = 0;
    }
    if (Key_Flag[2] == 1)
    {
        BUZZER_Middle_Flag = 1;
        if (DF.SMFlag == Err)
        {
            DF.ErrFlag = F_NOERR;
        }
        else if ((DF.SMFlag == Rise) || (DF.SMFlag == Run))
        {
            DF.OUTPUT_Flag = 0;
            DF.PWMENFlag = 0;
            HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
            HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
            DF.SMFlag = Wait;
            DF.BBFlag = NA;
        }
        else if (DF.SMFlag == Wait)
        {
            DF.OUTPUT_Flag = 1;
            DF.SMFlag = Rise;
        }
        USART1_Printf("KEY2 Pressed\r\n");
        Key_Flag[2] = 0;
    }
    if (Key_Flag[3] == 1)
    {
        BUZZER_Middle_Flag = 1;
        if (DF.SMFlag == Err)
        {
            DF.ErrFlag = F_NOERR;
        }
        else if (Screen_page == VIset_page || Screen_page == SET_page)
        {
            if (SET_Value.currentSetting != 0)
            {
                SET_Value.SET_bit++;
                if (SET_Value.SET_bit > 4)
                {
                    SET_Value.SET_bit = 0;
                }
            }
        }
        USART1_Printf("Encoder KEY Pressed\r\n");
        Key_Flag[3] = 0;
    }
}

void Encoder(void)
{
    if (Encoder_Flag == 1)
    {
        HAL_Delay(1);
        if (HAL_GPIO_ReadPin(Encoder_A_GPIO_Port, Encoder_A_Pin) == 0)
        {
            BUZZER_Short_Flag = 1;
            if (HAL_GPIO_ReadPin(Encoder_B_GPIO_Port, Encoder_B_Pin) == 1)
            {
                USART1_Printf("Encoder CCW\r\n");
                if (SET_Value.currentSetting == 0)
                {
                    Screen_page--;
                    if (Screen_page < VIset_page)
                    {
                        Screen_page = SET_page;
                    }
                }
                else if (Screen_page == VIset_page)
                {
                    if (SET_Value.currentSetting == 1)
                    {
                        if (SET_Value.SET_bit == 1) { SET_Value.Vout -= 10; if (SET_Value.Vout < 0.5) SET_Value.Vout += 10; }
                        else if (SET_Value.SET_bit == 2) { SET_Value.Vout -= 1; if (SET_Value.Vout < 0.5) SET_Value.Vout = 0.5; }
                        else if (SET_Value.SET_bit == 3) { SET_Value.Vout -= 0.1; if (SET_Value.Vout < 0.5) SET_Value.Vout += 0.1; }
                        else if (SET_Value.SET_bit == 4) { SET_Value.Vout -= 0.01; if (SET_Value.Vout < 0.5) SET_Value.Vout += 0.01; }
                        if (SET_Value.SET_bit != 0)
                        {
                            SET_Value.SET_modified_flag = 1;
                            CtrValue.Vout_SETref = SET_Value.Vout * (4.7F / 75.0F) / REF_3V3 * ADC_MAX_VALUE;
                            CtrValue.Iout_ref = SET_Value.Iout * 0.005F * (6200.0F / 100.0F) / REF_3V3 * ADC_MAX_VALUE;
                        }
                    }
                    else if (SET_Value.currentSetting == 2)
                    {
                        if (SET_Value.SET_bit == 1) { SET_Value.Iout -= 10; if (SET_Value.Iout < 0.01) SET_Value.Iout += 10; }
                        else if (SET_Value.SET_bit == 2) { SET_Value.Iout -= 1; if (SET_Value.Iout < 0.01) SET_Value.Iout = 0.01; }
                        else if (SET_Value.SET_bit == 3) { SET_Value.Iout -= 0.1; if (SET_Value.Iout < 0.01) SET_Value.Iout += 0.1; }
                        else if (SET_Value.SET_bit == 4) { SET_Value.Iout -= 0.01; if (SET_Value.Iout < 0.01) SET_Value.Iout += 0.01; }
                        if (SET_Value.SET_bit != 0)
                        {
                            SET_Value.SET_modified_flag = 1;
                            CtrValue.Vout_SETref = SET_Value.Vout * (4.7F / 75.0F) / REF_3V3 * ADC_MAX_VALUE;
                            CtrValue.Iout_ref = SET_Value.Iout * 0.005F * (6200.0F / 100.0F) / REF_3V3 * ADC_MAX_VALUE;
                        }
                    }
                }
                else if (Screen_page == SET_page)
                {
                    if (SET_Value.currentSetting == 1)
                    {
                        if (SET_Value.SET_bit == 1) { MAX_OTP_VAL -= 10; if (MAX_OTP_VAL < 40.0) MAX_OTP_VAL += 10; }
                        else if (SET_Value.SET_bit == 2) { MAX_OTP_VAL -= 1; if (MAX_OTP_VAL < 40.0) MAX_OTP_VAL = 40.0; }
                        else if (SET_Value.SET_bit == 3) { MAX_OTP_VAL -= 0.1; if (MAX_OTP_VAL < 40.0) MAX_OTP_VAL += 0.1; }
                        else if (SET_Value.SET_bit == 4) { MAX_OTP_VAL -= 0.01; if (MAX_OTP_VAL < 40.0) MAX_OTP_VAL += 0.01; }
                        if (SET_Value.SET_bit != 0) SET_Value.SET_modified_flag = 1;
                    }
                    else if (SET_Value.currentSetting == 2)
                    {
                        if (SET_Value.SET_bit == 1) { MAX_VOUT_OCP_VAL -= 10; if (MAX_VOUT_OCP_VAL < 0.01) MAX_VOUT_OCP_VAL += 10; }
                        else if (SET_Value.SET_bit == 2) { MAX_VOUT_OCP_VAL -= 1; if (MAX_VOUT_OCP_VAL < 0.01) MAX_VOUT_OCP_VAL = 0.01; }
                        else if (SET_Value.SET_bit == 3) { MAX_VOUT_OCP_VAL -= 0.1; if (MAX_VOUT_OCP_VAL < 0.01) MAX_VOUT_OCP_VAL += 0.1; }
                        else if (SET_Value.SET_bit == 4) { MAX_VOUT_OCP_VAL -= 0.01; if (MAX_VOUT_OCP_VAL < 0.01) MAX_VOUT_OCP_VAL += 0.01; }
                        if (SET_Value.SET_bit != 0) SET_Value.SET_modified_flag = 1;
                    }
                    else if (SET_Value.currentSetting == 3)
                    {
                        if (SET_Value.SET_bit == 1) { MAX_VOUT_OVP_VAL -= 10; if (MAX_VOUT_OVP_VAL < 0.5) MAX_VOUT_OVP_VAL += 10; }
                        else if (SET_Value.SET_bit == 2) { MAX_VOUT_OVP_VAL -= 1; if (MAX_VOUT_OVP_VAL < 0.5) MAX_VOUT_OVP_VAL = 0.5; }
                        else if (SET_Value.SET_bit == 3) { MAX_VOUT_OVP_VAL -= 0.1; if (MAX_VOUT_OVP_VAL < 0.5) MAX_VOUT_OVP_VAL += 0.1; }
                        else if (SET_Value.SET_bit == 4) { MAX_VOUT_OVP_VAL -= 0.01; if (MAX_VOUT_OVP_VAL < 0.5) MAX_VOUT_OVP_VAL += 0.01; }
                        if (SET_Value.SET_bit != 0) SET_Value.SET_modified_flag = 1;
                    }
                }
            }
            else if (HAL_GPIO_ReadPin(Encoder_B_GPIO_Port, Encoder_B_Pin) == 0)
            {
                USART1_Printf("Encoder CW\r\n");
                if (SET_Value.currentSetting == 0)
                {
                    Screen_page++;
                    if (Screen_page > SET_page)
                    {
                        Screen_page = VIset_page;
                    }
                }
                else if (Screen_page == VIset_page)
                {
                    if (SET_Value.currentSetting == 1)
                    {
                        if (SET_Value.SET_bit == 1) { SET_Value.Vout += 10.0F; if (SET_Value.Vout > 48.5) SET_Value.Vout -= 10.0F; }
                        else if (SET_Value.SET_bit == 2) { SET_Value.Vout += 1; if (SET_Value.Vout > 48.5) SET_Value.Vout = 48.5F; }
                        else if (SET_Value.SET_bit == 3) { SET_Value.Vout += 0.1F; if (SET_Value.Vout > 48.5) SET_Value.Vout -= 0.1F; }
                        else if (SET_Value.SET_bit == 4) { SET_Value.Vout += 0.01F; if (SET_Value.Vout > 48.5) SET_Value.Vout -= 0.01F; }
                        if (SET_Value.SET_bit != 0)
                        {
                            SET_Value.SET_modified_flag = 1;
                            CtrValue.Vout_SETref = SET_Value.Vout * (4.7F / 75.0F) / REF_3V3 * ADC_MAX_VALUE;
                            CtrValue.Iout_ref = SET_Value.Iout * 0.005F * (6200.0F / 100.0F) / REF_3V3 * ADC_MAX_VALUE;
                        }
                    }
                    else if (SET_Value.currentSetting == 2)
                    {
                        if (SET_Value.SET_bit == 1) { SET_Value.Iout += 10.0F; if (SET_Value.Iout > 10.1) SET_Value.Iout -= 10.0F; }
                        else if (SET_Value.SET_bit == 2) { SET_Value.Iout += 1; if (SET_Value.Iout > 10.1) SET_Value.Iout = 10.1F; }
                        else if (SET_Value.SET_bit == 3) { SET_Value.Iout += 0.1F; if (SET_Value.Iout > 10.1) SET_Value.Iout -= 0.1F; }
                        else if (SET_Value.SET_bit == 4) { SET_Value.Iout += 0.01F; if (SET_Value.Iout > 10.1) SET_Value.Iout -= 0.01F; }
                        if (SET_Value.SET_bit != 0)
                        {
                            SET_Value.SET_modified_flag = 1;
                            CtrValue.Vout_SETref = SET_Value.Vout * (4.7F / 75.0F) / REF_3V3 * ADC_MAX_VALUE;
                            CtrValue.Iout_ref = SET_Value.Iout * 0.005F * (6200.0F / 100.0F) / REF_3V3 * ADC_MAX_VALUE;
                        }
                    }
                }
                else if (Screen_page == SET_page)
                {
                    if (SET_Value.currentSetting == 1)
                    {
                        if (SET_Value.SET_bit == 1) { MAX_OTP_VAL += 10.0; if (MAX_OTP_VAL > 99.99) MAX_OTP_VAL -= 10; }
                        else if (SET_Value.SET_bit == 2) { MAX_OTP_VAL += 1.0; if (MAX_OTP_VAL > 99.99) MAX_OTP_VAL = 99.99; }
                        else if (SET_Value.SET_bit == 3) { MAX_OTP_VAL += 0.1; if (MAX_OTP_VAL > 99.99) MAX_OTP_VAL -= 0.1; }
                        else if (SET_Value.SET_bit == 4) { MAX_OTP_VAL += 0.01; if (MAX_OTP_VAL > 99.99) MAX_OTP_VAL -= 0.01; }
                        if (SET_Value.SET_bit != 0) SET_Value.SET_modified_flag = 1;
                    }
                    else if (SET_Value.currentSetting == 2)
                    {
                        if (SET_Value.SET_bit == 1) { MAX_VOUT_OCP_VAL += 10.0F; if (MAX_VOUT_OCP_VAL > 10.5) MAX_VOUT_OCP_VAL -= 10; }
                        else if (SET_Value.SET_bit == 2) { MAX_VOUT_OCP_VAL += 1.0F; if (MAX_VOUT_OCP_VAL > 10.5) MAX_VOUT_OCP_VAL = 10.5; }
                        else if (SET_Value.SET_bit == 3) { MAX_VOUT_OCP_VAL += 0.1F; if (MAX_VOUT_OCP_VAL > 10.5) MAX_VOUT_OCP_VAL -= 0.1; }
                        else if (SET_Value.SET_bit == 4) { MAX_VOUT_OCP_VAL += 0.01F; if (MAX_VOUT_OCP_VAL > 10.5) MAX_VOUT_OCP_VAL -= 0.01; }
                        if (SET_Value.SET_bit != 0) SET_Value.SET_modified_flag = 1;
                    }
                    else if (SET_Value.currentSetting == 3)
                    {
                        if (SET_Value.SET_bit == 1) { MAX_VOUT_OVP_VAL += 10.0F; if (MAX_VOUT_OVP_VAL > 50.0) MAX_VOUT_OVP_VAL -= 10; }
                        else if (SET_Value.SET_bit == 2) { MAX_VOUT_OVP_VAL += 1.0F; if (MAX_VOUT_OVP_VAL > 50.0) MAX_VOUT_OVP_VAL = 50.0; }
                        else if (SET_Value.SET_bit == 3) { MAX_VOUT_OVP_VAL += 0.1F; if (MAX_VOUT_OVP_VAL > 50.0) MAX_VOUT_OVP_VAL -= 0.1; }
                        else if (SET_Value.SET_bit == 4) { MAX_VOUT_OVP_VAL += 0.01; if (MAX_VOUT_OVP_VAL > 50.0) MAX_VOUT_OVP_VAL -= 0.01; }
                        if (SET_Value.SET_bit != 0) SET_Value.SET_modified_flag = 1;
                    }
                }
            }
            Encoder_Flag = 0;
        }
    }
}

CCMRAM void ADCSample(void)
{
    static uint32_t VinAvgSum = 0, IinAvgSum = 0, VoutAvgSum = 0, IoutAvgSum = 0;

    SADC.Vin = (uint32_t)ADC1_RESULT[0];
    SADC.Iin = (uint32_t)ADC1_RESULT[1];
    SADC.Vout = (uint32_t)((ADC1_RESULT[2] * CAL_VOUT_K >> 12) + CAL_VOUT_B);
    SADC.Iout = (uint32_t)((ADC1_RESULT[3] * CAL_IOUT_K >> 12) + CAL_IOUT_B);

    if (SADC.Vin < 15)
        SADC.Vin = 0;
    if (SADC.Vout < 15)
        SADC.Vout = 0;
    if (SADC.Iout < 16)
        SADC.Iout = 0;

    VinAvgSum = VinAvgSum + SADC.Vin - (VinAvgSum >> 3);
    SADC.VinAvg = VinAvgSum >> 3;
    IinAvgSum = IinAvgSum + SADC.Iin - (IinAvgSum >> 3);
    SADC.IinAvg = IinAvgSum >> 3;
    VoutAvgSum = VoutAvgSum + SADC.Vout - (VoutAvgSum >> 3);
    SADC.VoutAvg = VoutAvgSum >> 3;
    IoutAvgSum = IoutAvgSum + SADC.Iout - (IoutAvgSum >> 3);
    SADC.IoutAvg = IoutAvgSum >> 3;
}

void ADC_calculate(void)
{
    VIN = SADC.VinAvg * REF_3V3 / ADC_MAX_VALUE / (4.7F / 75.0F);
    IIN = SADC.IinAvg * REF_3V3 / ADC_MAX_VALUE / 62.0F / 0.005F;
    VOUT = SADC.VoutAvg * REF_3V3 / ADC_MAX_VALUE / (4.7F / 75.0F);
    IOUT = SADC.IoutAvg * REF_3V3 / ADC_MAX_VALUE / 62.0F / 0.005F;
    MainBoard_TEMP = GET_NTC_Temperature();
    CPU_TEMP = GET_CPU_Temperature();
}

void StateM(void)
{
    switch (DF.SMFlag)
    {
    case Init:
        StateMInit();
        break;
    case Wait:
        StateMWait();
        break;
    case Rise:
        StateMRise();
        break;
    case Run:
        StateMRun();
        break;
    case Err:
        StateMErr();
        break;
    }
}

void StateMInit(void)
{
    ValInit();
    DF.SMFlag = Wait;
}

void ValInit(void)
{
    DF.PWMENFlag = 0;
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
    DF.BBFlag = NA;
    DF.ErrFlag = 0;
    CtrValue.Vout_ref = 0;
    CtrValue.BuckDuty = MIN_BUKC_DUTY;
    CtrValue.BUCKMaxDuty = MIN_BUKC_DUTY;
    CtrValue.BoostDuty = MIN_BOOST_DUTY;
    CtrValue.BoostMaxDuty = MIN_BOOST_DUTY;
    VErr0 = 0;
    VErr1 = 0;
    VErr2 = 0;
    u0 = 0;
    u1 = 0;
    SET_Value.Vout = 5.0;
    SET_Value.Iout = 10.0;
    MAX_OTP_VAL = 80.0F;
    MAX_VOUT_OVP_VAL = 50.0F;
    MAX_VOUT_OCP_VAL = 10.5F;
}

void StateMRun(void)
{
}

void StateMErr(void)
{
    DF.PWMENFlag = 0;
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
    DF.BBFlag = NA;
    if (DF.ErrFlag == F_NOERR)
    {
        DF.SMFlag = Wait;
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
    }
}

void StateMWait(void)
{
    static uint16_t CntS = 0;
    static uint32_t IinSum = 0, IoutSum = 0;

    DF.PWMENFlag = 0;
    CntS++;
    if (CntS > 200)
    {
        CntS = 200;
        if ((DF.ErrFlag == F_NOERR) && (DF.OUTPUT_Flag == 1))
        {
            CntS = 0;
            IinSum = 0;
            IoutSum = 0;
            DF.SMFlag = Rise;
            STState = SSInit;
        }
    }
}

void StateMRise(void)
{
    static uint16_t Cnt = 0;
    static uint16_t BUCKMaxDutyCnt = 0, BoostMaxDutyCnt = 0;

    switch (STState)
    {
    case SSInit:
    {
        DF.PWMENFlag = 0;
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
        CtrValue.BUCKMaxDuty = MIN_BUKC_DUTY;
        CtrValue.BoostMaxDuty = MIN_BOOST_DUTY;
        VErr0 = 0;
        VErr1 = 0;
        VErr2 = 0;
        u0 = 0;
        u1 = 0;
        CtrValue.Vout_SETref = SET_Value.Vout * (4.7F / 75.0F) / REF_3V3 * ADC_MAX_VALUE;
        CtrValue.Iout_ref = SET_Value.Iout * 0.005F * (6200.0F / 100.0F) / REF_3V3 * ADC_MAX_VALUE;
        STState = SSWait;
        break;
    }
    case SSWait:
    {
        Cnt++;
        if (Cnt > 5)
        {
            Cnt = 0;
            CtrValue.BuckDuty = MIN_BUKC_DUTY;
            CtrValue.BUCKMaxDuty = MIN_BUKC_DUTY;
            CtrValue.BoostDuty = MIN_BOOST_DUTY;
            CtrValue.BoostMaxDuty = MIN_BOOST_DUTY;
            VErr0 = 0;
            VErr1 = 0;
            VErr2 = 0;
            u0 = 0;
            u1 = 0;
            CtrValue.Vout_SSref = CtrValue.Vout_SETref >> 1;
            STState = SSRun;
        }
        break;
    }
    case SSRun:
    {
        if (DF.PWMENFlag == 0)
        {
            VErr0 = 0;
            VErr1 = 0;
            VErr2 = 0;
            u0 = 0;
            u1 = 0;
            __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, 30000);
            __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F, HRTIM_COMPAREUNIT_1, 30000);
            HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
            HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
        }
        DF.PWMENFlag = 1;
        BUCKMaxDutyCnt++;
        BoostMaxDutyCnt++;
        CtrValue.BUCKMaxDuty = CtrValue.BUCKMaxDuty + BUCKMaxDutyCnt * 15;
        CtrValue.BoostMaxDuty = CtrValue.BoostMaxDuty + BoostMaxDutyCnt * 15;
        if (CtrValue.BUCKMaxDuty > MAX_BUCK_DUTY)
            CtrValue.BUCKMaxDuty = MAX_BUCK_DUTY;
        if (CtrValue.BoostMaxDuty > MAX_BOOST_DUTY)
            CtrValue.BoostMaxDuty = MAX_BOOST_DUTY;

        if ((CtrValue.BUCKMaxDuty == MAX_BUCK_DUTY) && (CtrValue.BoostMaxDuty == MAX_BOOST_DUTY))
        {
            DF.SMFlag = Run;
            STState = SSInit;
        }
        break;
    }
    default:
        break;
    }
}

void ShortOff(void)
{
    static int32_t RSCnt = 0;
    static uint8_t RSNum = 0;
    float Vout = SADC.Vout * REF_3V3 / ADC_MAX_VALUE / (4.7F / 75.0F);
    float Iout = SADC.Iout * REF_3V3 / ADC_MAX_VALUE / 62.0F / 0.005F;
    if ((Iout > MAX_SHORT_I) && (Vout < MIN_SHORT_V))
    {
        DF.PWMENFlag = 0;
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
        setRegBits(DF.ErrFlag, F_SW_SHORT);
        DF.SMFlag = Err;
    }
    if (getRegBits(DF.ErrFlag, F_SW_SHORT))
    {
        RSCnt++;
        if (RSCnt > 400)
        {
            RSCnt = 0;
            if (RSNum > 10)
            {
                RSNum = 11;
                DF.PWMENFlag = 0;
                HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
                HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
            }
            else
            {
                RSNum++;
                clrRegBits(DF.ErrFlag, F_SW_SHORT);
            }
        }
    }
}

void OVP(void)
{
    static uint16_t OVPCnt = 0;
    float Vout = SADC.Vout * REF_3V3 / ADC_MAX_VALUE / (4.7F / 75.0F);
    if (Vout >= MAX_VOUT_OVP_VAL)
    {
        OVPCnt++;
        if (OVPCnt > 2)
        {
            OVPCnt = 0;
            DF.PWMENFlag = 0;
            HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
            HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
            setRegBits(DF.ErrFlag, F_SW_VOUT_OVP);
            DF.SMFlag = Err;
        }
    }
    else
        OVPCnt = 0;
}

void OCP(void)
{
    static uint16_t OCPCnt = 0;
    static uint16_t RSCnt = 0;
    static uint16_t RSNum = 0;

    float Iout = SADC.Iout * REF_3V3 / ADC_MAX_VALUE / 62.0F / 0.005F;

    if ((Iout >= MAX_VOUT_OCP_VAL) && (DF.SMFlag == Run))
    {
        OCPCnt++;
        if (OCPCnt > 10)
        {
            OCPCnt = 0;
            DF.PWMENFlag = 0;
            HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
            HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
            setRegBits(DF.ErrFlag, F_SW_IOUT_OCP);
            DF.SMFlag = Err;
        }
    }
    else
        OCPCnt = 0;

    if (getRegBits(DF.ErrFlag, F_SW_IOUT_OCP))
    {
        RSCnt++;
        if (RSCnt > 400)
        {
            RSCnt = 0;
            RSNum++;
            if (RSNum > 10)
            {
                RSNum = 11;
                DF.PWMENFlag = 0;
                HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
                HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
            }
            else
            {
                clrRegBits(DF.ErrFlag, F_SW_IOUT_OCP);
            }
        }
    }
}

void OTP(void)
{
    float TEMP = GET_NTC_Temperature();
    if (TEMP >= MAX_OTP_VAL)
    {
        DF.SMFlag = Wait;
        DF.PWMENFlag = 0;
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
        setRegBits(DF.ErrFlag, F_OTP);
        DF.SMFlag = Err;
    }
}

CCMRAM void BBMode(void)
{
    uint8_t PreBBFlag = 0;
    PreBBFlag = DF.BBFlag;

    uint32_t VIN_ADC = ADC1_RESULT[0];
    static uint32_t VIN_ADC_SUM = 0;
    static uint8_t VIN_ADC_Count = 0;

    if (VIN_ADC_Count < 5)
    {
        VIN_ADC_SUM += ADC1_RESULT[0];
        VIN_ADC_Count++;
    }
    if (VIN_ADC_Count == 5)
    {
        VIN_ADC = VIN_ADC_SUM / 5;
        VIN_ADC_SUM = 0;
        VIN_ADC_Count = 0;
    }

    switch (DF.BBFlag)
    {
    case NA:
    {
        if (CtrValue.Vout_ref < (VIN_ADC * 0.8F))
            DF.BBFlag = Buck;
        else if (CtrValue.Vout_ref > (VIN_ADC * 1.2F))
            DF.BBFlag = Boost;
        else
            DF.BBFlag = Mix;
        break;
    }
    case Buck:
    {
        if (CtrValue.Vout_ref > (VIN_ADC * 1.2F))
            DF.BBFlag = Boost;
        else if (CtrValue.Vout_ref > (VIN_ADC * 0.85F))
            DF.BBFlag = Mix;
        break;
    }
    case Boost:
    {
        if (CtrValue.Vout_ref < ((VIN_ADC * 0.8F)))
            DF.BBFlag = Buck;
        else if (CtrValue.Vout_ref < (VIN_ADC * 1.15F))
            DF.BBFlag = Mix;
        break;
    }
    case Mix:
    {
        if (CtrValue.Vout_ref < (VIN_ADC * 0.8F))
            DF.BBFlag = Buck;
        else if (CtrValue.Vout_ref > (VIN_ADC * 1.2F))
            DF.BBFlag = Boost;
        break;
    }
    }

    if (PreBBFlag == DF.BBFlag)
        DF.BBModeChange = 0;
    else
        DF.BBModeChange = 1;
}

void BUZZER_Short(void)
{
    if (BUZZER_Flag == 1 && BUZZER_Middle_Flag == 0)
    {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
        BUZZER_Flag = 0;
        BUZZER_Short_Flag = 0;
    }
    else if (BUZZER_Short_Flag == 1)
    {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        BUZZER_Flag = 1;
    }
}

void BUZZER_Middle(void)
{
    if (BUZZER_Flag == 1)
    {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
        BUZZER_Flag = 0;
        BUZZER_Middle_Flag = 0;
    }
    else if (BUZZER_Middle_Flag == 1)
    {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        BUZZER_Flag = 1;
    }
}

float one_order_lowpass_filter(float input, float alpha)
{
    static float prev_output = 0.0F;
    float output = alpha * input + (1.0F - alpha) * prev_output;
    prev_output = output;
    return output;
}

float calculateTemperature(float voltage)
{
    float Rt = 0;
    float R = 10000;
    float T0 = 273.15F + 25;
    float B = 3950;
    float Ka = 273.15F;
    Rt = (REF_3V3 - voltage) * 10000.0F / voltage;
    float temperature = 1.0F / (1.0F / T0 + log(Rt / R) / B) - Ka;
    return temperature;
}

float GET_NTC_Temperature(void)
{
    HAL_ADC_Start(&hadc2);
    uint32_t TEMP_adcValue = HAL_ADC_GetValue(&hadc2);
    float temperature = calculateTemperature(TEMP_adcValue * REF_3V3 / 65520.0F);
    return temperature;
}

float GET_CPU_Temperature(void)
{
    HAL_ADC_Start(&hadc5);
    float Temp_Scale = (float)(TS_CAL2_TEMP - TS_CAL1_TEMP) / (float)(TS_CAL2 - TS_CAL1);
    float TEMP_adcValue = HAL_ADC_GetValue(&hadc5) / 8.0F;
    float temperature = Temp_Scale * (TEMP_adcValue * (REF_3V3 / 3.0F) - TS_CAL1) + TS_CAL1_TEMP;
    return one_order_lowpass_filter(temperature, 0.1F);
}

void FAN_PWM_set(uint16_t dutyCycle)
{
    if (dutyCycle > 100)
    {
        dutyCycle = 100;
    }
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, dutyCycle * 10);
}

void Auto_FAN(void)
{
    float TEMP = GET_NTC_Temperature();
    if (TEMP < 35)          FAN_PWM_set(0);
    else if (TEMP >= 35)    FAN_PWM_set(35);
    else if (TEMP >= 40)    FAN_PWM_set(45);
    else if (TEMP >= 45)    FAN_PWM_set(60);
    else if (TEMP >= 50)    FAN_PWM_set(70);
    else if (TEMP >= 55)    FAN_PWM_set(80);
    else if (TEMP >= 60)    FAN_PWM_set(90);
    else if (TEMP >= 65)    FAN_PWM_set(100);
}
