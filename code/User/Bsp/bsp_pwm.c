/**
 * @file bsp_pwm.c
 * @brief PWM 驱动实现 - 基于 GTM ATOM 的 PWM 输出
 */

#include "bsp_pwm.h"

#pragma section all "cpu0_dsram"

/*==============================================================================
 * PWM 设备定义
 *============================================================================*/

/* 示例 PWM 设备 */
struct BspPwm bsp_pwm_servo1;    /* 舵机 PWM */
struct BspPwm bsp_pwm_motor;     /* 电机 PWM */

#pragma section all restore

/*==============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化 PWM 设备
 * @param pwm PWM 结构体指针
 * @param channel PWM 通道引脚
 * @param freq PWM 频率 (Hz)
 * @param duty 初始占空比
 */
void bsp_pwm_init(BspPwm *pwm, pwm_channel_enum channel, uint32 freq, uint32 duty)
{
    pwm->channel   = channel;
    pwm->frequency = freq;
    pwm->duty     = duty;

    /* 初始化逐飞 PWM 驱动 */
    pwm_init(channel, freq, duty);
}

/**
 * @brief 设置 PWM 占空比
 * @param pwm PWM 结构体指针
 * @param duty 占空比值
 */
void bsp_pwm_set_duty(BspPwm *pwm, uint32 duty)
{
    pwm->duty = duty;
    pwm_set_duty(pwm->channel, duty);
}

/**
 * @brief 获取当前 PWM 占空比
 * @param pwm PWM 结构体指针
 * @return 当前占空比值
 */
uint32 bsp_pwm_get_duty(BspPwm *pwm)
{
    return pwm->duty;
}

/**
 * @brief 关闭 PWM 输出
 * @param pwm PWM 结构体指针
 */
void bsp_pwm_stop(BspPwm *pwm)
{
    pwm_set_duty(pwm->channel, 0);
    pwm->duty = 0;
}

/**
 * @brief 关闭所有 PWM 通道
 */
void bsp_pwm_all_stop(void)
{
    pwm_all_channel_close();
}

/**
 * @brief 总初始化函数
 * @note 示例初始化多个 PWM 通道
 */
void bsp_pwm_all_init(void)
{
    /* 舵机 PWM - ATOM0_CH3_P21_5, 330Hz, 初始占空比 4950 (中值1500μs) */
    bsp_pwm_init(&bsp_pwm_servo1, ATOM0_CH3_P21_5, 330, 4950);

    /* 电机 PWM - ATOM1_CH0_P21_2, 1000Hz, 初始占空比 0 */
    bsp_pwm_init(&bsp_pwm_motor, ATOM1_CH0_P21_2, 1000, 0);
}
