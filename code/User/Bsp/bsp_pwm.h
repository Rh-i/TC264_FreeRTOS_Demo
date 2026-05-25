#ifndef BSP_PWM_H_
#define BSP_PWM_H_

#include "zf_common_headfile.h"
#include "zf_driver_pwm.h"

/*==============================================================================
 * PWM 设备定义
 * @brief 支持多通道 PWM 输出，可配置频率和占空比
 *============================================================================*/
typedef struct BspPwm
{
  pwm_channel_enum channel;   /**< PWM 通道引脚 */
  uint32           frequency; /**< PWM 频率 (Hz) */
  uint32           duty;      /**< 当前占空比值 (0-10000) */
} BspPwm;

extern struct BspPwm bsp_pwm_servo1; /* 舵机 PWM */
extern struct BspPwm bsp_pwm_motor;  /* 电机 PWM */


/*==============================================================================
 * 公共函数声明
 *============================================================================*/

/**
 * @brief 初始化 PWM 设备
 * @param pwm PWM 结构体指针
 * @param channel PWM 通道引脚
 * @param freq PWM 频率 (Hz)
 * @param duty 初始占空比 (0-10000，对应 0%-100%)
 */
void bsp_pwm_init(BspPwm *pwm, pwm_channel_enum channel, uint32 freq, uint32 duty);

/**
 * @brief 设置 PWM 占空比
 * @param pwm PWM 结构体指针
 * @param duty 占空比值 (0-10000)
 */
void bsp_pwm_set_duty(BspPwm *pwm, uint32 duty);

/**
 * @brief 获取当前 PWM 占空比
 * @param pwm PWM 结构体指针
 * @return 当前占空比值
 */
uint32 bsp_pwm_get_duty(BspPwm *pwm);

/**
 * @brief 关闭 PWM 输出
 * @param pwm PWM 结构体指针
 */
void bsp_pwm_stop(BspPwm *pwm);

/**
 * @brief 关闭所有 PWM 通道
 */
void bsp_pwm_all_stop(void);

/**
 * @brief 总初始化函数（示例）
 */
void bsp_pwm_all_init(void);

#endif /* BSP_PWM_H_ */
