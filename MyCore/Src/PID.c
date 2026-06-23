#include "PID.h"
#include "hrtim.h"
#include "function.h"

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
/*

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

*/

CCMRAM void BuckBoostVILoopCtlPID(void)
{
    static int32_t I_Integral = 0; // 电流环路积分量

    CtrValue.Vout_ref = CtrValue.Vout_SETref; // 输出参考电压设置为设置电压

    int32_t VoutTemp = (ADC1_RESULT[2] * CAL_VOUT_K >> 12) + CAL_VOUT_B; // 获取矫正后的输出电压
    int32_t IoutTemp = (ADC1_RESULT[3] * CAL_IOUT_K >> 12) + CAL_IOUT_B; // 获取矫正后的输出电流

    // 计算电流误差量，当输出电流小于参考电流，输出量增加
    IErr0 = CtrValue.Iout_ref - IoutTemp;
    // 电流环路输出= 积分量 + KP*误差量 + KD*当前误差减上次误差
    i0 = I_Integral + IErr0 * ILOOP_KP + (IErr0 - IErr1) * ILOOP_KD;
    // 积分量=积分量+KI*误差量
    I_Integral = I_Integral + IErr0 * ILOOP_KI;

    // 积分量限制，积分量最大值限制
    if (I_Integral > ADC_MAX_VALUE)
        I_Integral = ADC_MAX_VALUE;

    if (DF.SMFlag == Rise && (VoutTemp < (CtrValue.Vout_ref / 2))) // 判断是否在软启动状态
    {

        CtrValue.Vout_ref = CtrValue.Vout_ref + i0;  // 输出参考电压加上电流环计算结果
        CVCC_Mode = CC;                              // 恒流模式
        if (CtrValue.Vout_ref > CtrValue.Vout_SSref) // 输出参考电压超过软启动设置电压时限制在软启动设置电压
        {
            CtrValue.Vout_ref = CtrValue.Vout_SSref; // 限制输出参考电压
            CVCC_Mode = CV;                          // 恒压模式
        }
        if (CtrValue.Vout_ref < 0) // 输出参考电压小于0时限制在0
        {
            CtrValue.Vout_ref = 0;
        }
    }
    else
    {
        CtrValue.Vout_ref = CtrValue.Vout_ref + i0;   // 输出参考电压加上电流环计算结果
        CVCC_Mode = CC;                               // 恒流模式
        if (CtrValue.Vout_ref > CtrValue.Vout_SETref) // 输出参考电压超过设置电压时限制在设置电压
        {
            CtrValue.Vout_ref = CtrValue.Vout_SETref; // 限制输出参考电压
            CVCC_Mode = CV;                           // 恒压模式
        }
        if (CtrValue.Vout_ref < 0) // 输出参考电压小于0时限制在0
        {
            CtrValue.Vout_ref = 0;
        }
    }

    VErr0 = CtrValue.Vout_ref - VoutTemp; // 计算电压误差量，当参考电压大于输出电压，占空比增加，输出量增加

    // 当模式切换时，降低占空比，确保模式切换不过冲
    // BBModeChange为模式切换为，不同模式切换时，该位会被置1
    if (DF.BBModeChange)
    {
        u1 = 0;
        I_Integral = 0;
        i0 = 0;
        DF.BBModeChange = 0;
    }

    // 判断工作模式，BUCK，BOOST，BUCK-BOOST
    switch (DF.BBFlag)
    {
    case NA: // 初始阶段
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
        break;
    }
    case Buck: // BUCK模式
    {
        u0 = u1 + VErr0 * BUCKPIDb0 + VErr1 * BUCKPIDb1 + VErr2 * BUCKPIDb2; // 计算电压环输出
        // 历史数据幅值
        VErr2 = VErr1;
        VErr1 = VErr0;
        u1 = u0;

        // 环路输出赋值
        CtrValue.BoostDuty = MIN_BOOST_DUTY1; // BOOST上管固定占空比94%，下管6%
        CtrValue.BuckDuty = (u0 >> 8) * 3;    // 电压环占空比输出

        // 环路输出最大最小占空比限制
        if (CtrValue.BuckDuty > CtrValue.BUCKMaxDuty)
            CtrValue.BuckDuty = CtrValue.BUCKMaxDuty;
        if (CtrValue.BuckDuty < MIN_BUKC_DUTY)
            CtrValue.BuckDuty = MIN_BUKC_DUTY;
        break;
    }
    case Boost: // Boost模式
    {
        // 调用PID环路计算公式（参照PID环路计算文档）
        u0 = u1 + VErr0 * BOOSTPIDb0 + VErr1 * BOOSTPIDb1 + VErr2 * BOOSTPIDb2;
        // 历史数据幅值
        VErr2 = VErr1;
        VErr1 = VErr0;
        u1 = u0;

        // 环路输出赋值
        CtrValue.BuckDuty = MAX_BUCK_DUTY;  // BUCK上管固定占空比94%
        CtrValue.BoostDuty = (u0 >> 8) * 3; // 电压环占空比输出

        // 环路输出最大最小占空比限制
        if (CtrValue.BoostDuty > CtrValue.BoostMaxDuty)
            CtrValue.BoostDuty = CtrValue.BoostMaxDuty;
        if (CtrValue.BoostDuty < MIN_BOOST_DUTY)
            CtrValue.BoostDuty = MIN_BOOST_DUTY;
        break;
    }
    case Mix: // Mix模式
    {
        // 调用PID环路计算公式
        u0 = u1 + VErr0 * BOOSTPIDb0 + VErr1 * BOOSTPIDb1 + VErr2 * BOOSTPIDb2;
        // 历史数据幅值
        VErr2 = VErr1;
        VErr1 = VErr0;
        u1 = u0;
        IErr1 = IErr0;

        // 环路输出赋值
        CtrValue.BuckDuty = MAX_BUCK_DUTY1; // BUCK上管固定占空比80%
        CtrValue.BoostDuty = (u0 >> 8) * 3; // 电压环占空比输出

        // 环路输出最大最小占空比限制
        if (CtrValue.BoostDuty > CtrValue.BoostMaxDuty)
            CtrValue.BoostDuty = CtrValue.BoostMaxDuty;
        if (CtrValue.BoostDuty < MIN_BOOST_DUTY)
            CtrValue.BoostDuty = MIN_BOOST_DUTY;
        break;
    }
    }

    // PWMENFlag是PWM开启标志位，当该位为0时,buck的占空比为0，无输出;
    if (DF.PWMENFlag == 0)
        CtrValue.BuckDuty = MIN_BUKC_DUTY;

    // 更新对应寄存器
    // buck占空比
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, PERIOD - CtrValue.BuckDuty);
    // ADC触发采样点，buck占空比的一半，右移1位为除以2
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_3, __HAL_HRTIM_GETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1) >> 1);
    // Boost占空比
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F, HRTIM_COMPAREUNIT_1, CtrValue.BoostDuty);
}
