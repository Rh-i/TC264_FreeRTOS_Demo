#ifndef __R9DS_CTRL_H__
#define __R9DS_CTRL_H__

#include "device_motor.h"
#include "device_r9ds.h"
#include "device_servo.h"
#include "zf_common_headfile.h"

/*==============================================================================
 * 可配置宏 — 遥控器参数
 *============================================================================*/

// 死区范围（在此范围内视为中性"0"位置）
#define R9DS_DEADZONE_LOW  980U
#define R9DS_DEADZONE_HIGH 1020U

// 通道值范围
#define R9DS_CH_MIN  200U   // 通道最小值
#define R9DS_CH_MAX  1800U  // 通道最大值
#define R9DS_CH_MID  1000U  // 通道中位值（摇杆/旋钮中位）

// 电机/舵机物理限值
#define R9DS_MOTOR_MAX_SPEED 150  // CH1 满量程对应电机速度 (cm/s)
#define R9DS_SERVO_MAX_ANGLE 35   // CH2 满量程对应舵机角度 (°)

/*==============================================================================
 * 控制模式枚举
 * @brief 由 S3 开关 + 遥控器在线状态决定
 *============================================================================*/
typedef enum
{
  R9DS_CTRL_MODE_MANUAL = 1, // 手动模式：仅遥控器控制，屏蔽自动控制
  R9DS_CTRL_MODE_NONE   = 2, // 无控制模式：屏蔽所有控制，立即刹停 + 舵机回中
  R9DS_CTRL_MODE_AUTO   = 3, // 自动模式：仅自动控制（串口协议），屏蔽遥控器（默认值）
} R9DS_CtrlMode;

/*==============================================================================
 * 公共接口
 *============================================================================*/

/**
 * @brief 初始化遥控器控制模块
 * @note 默认模式 = AUTO，在 user_init 中调用
 */
void r9ds_ctrl_init(void);

/**
 * @brief 获取当前有效控制模式
 * @return R9DS_CtrlMode 枚举值
 * @note 当遥控器离线时强制返回 AUTO
 */
R9DS_CtrlMode r9ds_ctrl_get_mode(void);

/**
 * @brief 遥控器控制更新（20ms ISR 调用）
 * @note 内部根据当前模式执行：
 *       NONE   → device_motor_stop + device_servo_reset
 *       MANUAL → CH1 油门控制电机 / CH2 转向控制舵机 / S1 滑行或刹停
 *       AUTO   → 不执行任何操作（由 auto_ctrl 接管）
 */
void r9ds_ctrl_update(void);

#endif // __R9DS_CTRL_H__
