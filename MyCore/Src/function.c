#include "function.h"
#include "W25Q64.h"
#include <string.h>

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
volatile float MAX_OTP_VAL = 80.0F;
volatile float MAX_VOUT_OVP_VAL = 50.0F;
volatile float MAX_VOUT_OCP_VAL = 10.5F;

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
