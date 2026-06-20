#include "PID.h"
#include "hrtim.h"
#include "function.h"
#include "task.h"

#define CCMRAM __attribute__((section("ccmram")))

extern volatile uint16_t ADC1_RESULT[4];
volatile int32_t VErr0 = 0, VErr1 = 0, VErr2 = 0;
volatile int32_t IErr0 = 0, IErr1 = 0;
volatile int32_t u0 = 0, u1 = 0;
volatile int32_t i0 = 0, i1 = 0;
volatile _CVCC_Mode CVCC_Mode = CV;

void PID_Init(void)
{
    VErr0 = 0;
    VErr1 = 0;
    VErr2 = 0;
    u0 = 0;
    u1 = 0;
    i0 = 0;
    IErr0 = 0;
}

#define BUCKPIDb0 5271
#define BUCKPIDb1 -10363
#define BUCKPIDb2 5093

#define BOOSTPIDb0 8044
#define BOOSTPIDb1 -15813
#define BOOSTPIDb2 7772

#define ILOOP_KP 6
#define ILOOP_KI 3
#define ILOOP_KD 1

CCMRAM void BuckBoostVILoopCtlPID_TEST(void){
    
}

CCMRAM void BuckBoostVILoopCtlPID(void)
{
    static int32_t I_Integral = 0;

    CtrValue.Vout_ref = CtrValue.Vout_SETref;

    int32_t VoutTemp = (ADC1_RESULT[2] * CAL_VOUT_K >> 12) + CAL_VOUT_B;
    int32_t IoutTemp = (ADC1_RESULT[3] * CAL_IOUT_K >> 12) + CAL_IOUT_B;

    IErr0 = CtrValue.Iout_ref - IoutTemp;
    i0 = I_Integral + IErr0 * ILOOP_KP + (IErr0 - IErr1) * ILOOP_KD;
    I_Integral = I_Integral + IErr0 * ILOOP_KI;

    if (I_Integral > (int32_t)ADC_MAX_VALUE){
        I_Integral = (int32_t)ADC_MAX_VALUE;
    }
    if (I_Integral < -(int32_t)ADC_MAX_VALUE){
        I_Integral = -(int32_t)ADC_MAX_VALUE;
    }

    if (DF.SMFlag == Rise && (VoutTemp < (CtrValue.Vout_ref / 2))){
        CtrValue.Vout_ref = CtrValue.Vout_ref + i0;
        CVCC_Mode = CC;
        if (CtrValue.Vout_ref > CtrValue.Vout_SSref)
        {
            CtrValue.Vout_ref = CtrValue.Vout_SSref;
            CVCC_Mode = CV;
        }
        if (CtrValue.Vout_ref < 0)
        {
            CtrValue.Vout_ref = 0;
        }
    }
    else{
        CtrValue.Vout_ref = CtrValue.Vout_ref + i0;
        CVCC_Mode = CC;
        if (CtrValue.Vout_ref > CtrValue.Vout_SETref)
        {
            CtrValue.Vout_ref = CtrValue.Vout_SETref;
            CVCC_Mode = CV;
        }
        if (CtrValue.Vout_ref < 0)
        {
            CtrValue.Vout_ref = 0;
        }
    }

    VErr0 = CtrValue.Vout_ref - VoutTemp;

    if (DF.BBModeChange){
        u1 = 0;
        I_Integral = 0;
        i0 = 0;
        DF.BBModeChange = 0;
    }

    switch (DF.BBFlag){
        case NA:
        {
            VErr0 = 0;
            VErr1 = 0;
            VErr2 = 0;
            u0 = 0;
            u1 = 0;
            i0 = 0;
            I_Integral = 0;
            IErr0 = 0;
            IErr1 = 0;
            CtrValue.BuckDuty = MIN_BUKC_DUTY;
            CtrValue.BoostDuty = MIN_BOOST_DUTY;
            break;
        }
        case Buck:
        {
            u0 = u1 + VErr0 * BUCKPIDb0 + VErr1 * BUCKPIDb1 + VErr2 * BUCKPIDb2;
            VErr2 = VErr1;
            VErr1 = VErr0;
            u1 = u0;

            CtrValue.BoostDuty = MIN_BOOST_DUTY1;
            CtrValue.BuckDuty = (u0 >> 8) * 3;

            if (CtrValue.BuckDuty > CtrValue.BUCKMaxDuty)
                CtrValue.BuckDuty = CtrValue.BUCKMaxDuty;
            if (CtrValue.BuckDuty < MIN_BUKC_DUTY)
                CtrValue.BuckDuty = MIN_BUKC_DUTY;
            break;
        }
        case Boost:
        {
            u0 = u1 + VErr0 * BOOSTPIDb0 + VErr1 * BOOSTPIDb1 + VErr2 * BOOSTPIDb2;
            VErr2 = VErr1;
            VErr1 = VErr0;
            u1 = u0;

            CtrValue.BuckDuty = MAX_BUCK_DUTY;
            CtrValue.BoostDuty = (u0 >> 8) * 3;

            if (CtrValue.BoostDuty > CtrValue.BoostMaxDuty)
                CtrValue.BoostDuty = CtrValue.BoostMaxDuty;
            if (CtrValue.BoostDuty < MIN_BOOST_DUTY)
                CtrValue.BoostDuty = MIN_BOOST_DUTY;
            break;
        }
        case Mix:
        {
            u0 = u1 + VErr0 * BOOSTPIDb0 + VErr1 * BOOSTPIDb1 + VErr2 * BOOSTPIDb2;
            VErr2 = VErr1;
            VErr1 = VErr0;
            u1 = u0;
            IErr1 = IErr0;

            CtrValue.BuckDuty = MAX_BUCK_DUTY1;
            CtrValue.BoostDuty = (u0 >> 8) * 3;

            if (CtrValue.BoostDuty > CtrValue.BoostMaxDuty)
                CtrValue.BoostDuty = CtrValue.BoostMaxDuty;
            if (CtrValue.BoostDuty < MIN_BOOST_DUTY)
                CtrValue.BoostDuty = MIN_BOOST_DUTY;
            break;
        }
    }

    if (DF.PWMENFlag == 0){ CtrValue.BuckDuty = MIN_BUKC_DUTY; }

    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, PERIOD - CtrValue.BuckDuty);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_3, __HAL_HRTIM_GETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1) >> 1);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F, HRTIM_COMPAREUNIT_1, CtrValue.BoostDuty);
}
