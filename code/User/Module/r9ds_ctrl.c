/**
 * @file r9ds_ctrl.c
 * @author Rh (qq:750920400)
 * @brief 遥控器控制模块 — R9DS SBUS 通道解析 → 电机/舵机控制
 * @version 0.1
 * @date 2026-05-27
 *
 * @note 由 20ms ISR (cc61_pit_ch1_isr) 周期性调用 r9ds_ctrl_update()
 *       S3 模式判定在应用层（本模块内部），离线自动回退到 AUTO
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "r9ds_ctrl.h"
#include "bsp_io.h"

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 模块内部状态
 *============================================================================*/
static R9DS_CtrlMode s_mode = R9DS_CTRL_MODE_AUTO; // 默认自动模式

/*==============================================================================
 * 私有函数声明
 *============================================================================*/

/**
 * @brief 死区检测 — 通道值在 [low, high] 内视为中性
 * @return 1=在死区内, 0=在死区外
 */
static inline uint8 r9ds_in_deadzone(uint16 val);

/**
 * @brief CH1 通道值 → 电机目标速度 (cm/s)
 * @param ch1 R9DS rc.ch1 原始值
 * @return 目标速度 (cm/s)，正=前进，负=后退
 */
static int32 r9ds_ch1_to_speed(uint16 ch1);

/**
 * @brief CH2 通道值 → 舵机目标角度 (°)
 * @param ch2 R9DS rc.ch2 原始值
 * @return 目标角度 (°)，正=右转，负=左转
 */
static int32 r9ds_ch2_to_angle(uint16 ch2);

/**
 * @brief 手动模式 — 油门控制
 * @note 读取 rc.ch1 + rc.s1，执行电机速度控制或滑行/刹停
 */
static void r9ds_manual_throttle(void);

/**
 * @brief 手动模式 — 转向控制
 * @note 读取 rc.ch2，执行舵机角度控制
 */
static void r9ds_manual_steering(void);

/*==============================================================================
 * 私有函数实现
 *============================================================================*/

static inline uint8 r9ds_in_deadzone(uint16 val)
{
  return (uint8)(val >= R9DS_DEADZONE_LOW && val <= R9DS_DEADZONE_HIGH);
}

/**
 * @brief CH1 → 电机速度线性映射
 * @details 映射公式（对称双向）：
 *          死区内    → speed = 0
 *          ch1 > 1000 → speed = +(ch1 - 1000) / 800 * 150  (前进)
 *          ch1 < 1000 → speed = -(1000 - ch1) / 800 * 150  (后退)
 *          范围: -150 ~ +150 cm/s
 */
static int32 r9ds_ch1_to_speed(uint16 ch1)
{
  if (r9ds_in_deadzone(ch1))
  {
    return 0;
  }

  int32 diff = (int32)ch1 - (int32)R9DS_CH_MID; // [-800, +800]
  // diff / 800 * 150 = diff * 150 / 800 = diff * 3 / 16
  return (diff * (int32)R9DS_MOTOR_MAX_SPEED) / (int32)(R9DS_CH_MAX - R9DS_CH_MID);
}

/**
 * @brief CH2 → 舵机角度线性映射
 * @details 映射公式：
 *          死区内    → angle = 0
 *          ch2 > 1000 → angle = -(ch2 - 1000) / 800 * 35  (左转)
 *          ch2 < 1000 → angle = +(1000 - ch2) / 800 * 35  (右转)
 *          范围: -35° ~ +35°
 */
static int32 r9ds_ch2_to_angle(uint16 ch2)
{
  if (r9ds_in_deadzone(ch2))
  {
    return 0;
  }

  int32 diff = (int32)R9DS_CH_MID - (int32)ch2; // 反转: ch2>1000→负(左转), ch2<1000→正(右转)
  // diff / 800 * 35
  return (diff * (int32)R9DS_SERVO_MAX_ANGLE) / (int32)(R9DS_CH_MAX - R9DS_CH_MID);
}

/**
 * @brief 手动模式油门控制
 * @note 死区外 → device_motor_set_speed (PID 闭环调速)
 *       死区内 + S1=1(滑行) → device_motor_stop (PWM=0, PID 复位)
 *       死区内 + S1=2(刹停) → device_motor_set_speed(0) (PID 主动制动到 0)
 */
static void r9ds_manual_throttle(void)
{
  uint16 ch1 = (uint16)g_r9ds.rc.ch1;
  uint8  s1  = (uint8)g_r9ds.rc.s1;

  if (r9ds_in_deadzone(ch1))
  {
    // 油门摇杆在中位 — 根据 S1 决定滑行或刹停
    if (s1 == 1)
    {
      // 滑行模式：PWM=0，电机自由减速
      device_motor_stop(&g_motor);
    }
    else // s1 == 2
    {
      // 刹停模式：PID 闭环制动到 0
      device_motor_set_speed(&g_motor, 0);
    }
  }
  else
  {
    // 油门摇杆不在死区 — 按比例输出速度
    int32 speed = r9ds_ch1_to_speed(ch1);
    device_motor_set_speed(&g_motor, speed);
  }
}

/**
 * @brief 手动模式转向控制
 */
static void r9ds_manual_steering(void)
{
  uint16 ch2   = (uint16)g_r9ds.rc.ch2;
  int32  angle = r9ds_ch2_to_angle(ch2);
  device_servo_set_angle(&g_servo, angle);
}

/*==============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化遥控器控制模块
 */
void r9ds_ctrl_init(void)
{
  s_mode = R9DS_CTRL_MODE_AUTO; // 默认自动模式
}

/**
 * @brief 获取当前有效控制模式
 * @note 模式判定规则：
 *       1. 遥控器离线 → 强制 AUTO（安全回退）
 *       2. rc.s3 == 1 → MANUAL（手动模式）
 *       3. rc.s3 == 2 → NONE（无控制模式）
 *       4. rc.s3 == 3 或其他 → AUTO（自动模式，默认值）
 */
R9DS_CtrlMode r9ds_ctrl_get_mode(void)
{
  // 遥控器离线 → 安全回退到自动模式
  if (!device_r9ds_is_online(&g_r9ds))
  {
    s_mode = R9DS_CTRL_MODE_AUTO;
    return s_mode;
  }

  // 根据 S3 开关位置判定模式
  switch (g_r9ds.rc.s3)
  {
    case 1:
      s_mode = R9DS_CTRL_MODE_MANUAL;
      break;
    case 2:
      s_mode = R9DS_CTRL_MODE_NONE;
      break;
    case 3:
    default:
      s_mode = R9DS_CTRL_MODE_AUTO;
      break;
  }

  return s_mode;
}

/**
 * @brief 遥控器控制更新 — 20ms ISR 中调用
 */
void r9ds_ctrl_update(void)
{
  R9DS_CtrlMode mode = r9ds_ctrl_get_mode();

  switch (mode)
  {
    case R9DS_CTRL_MODE_NONE:
      // 无控制模式：立即刹停 + 舵机回中
      device_motor_set_speed(&g_motor, 0);
      device_servo_reset(&g_servo);
      break;

    case R9DS_CTRL_MODE_MANUAL:
      // 手动模式：遥控器接管油门和转向
      r9ds_manual_throttle();
      r9ds_manual_steering();

      // S2 蜂鸣器控制：S2=2→蜂鸣器叫, S2=1→蜂鸣器不叫
      if (g_r9ds.rc.s2 == 2)
      {
        buzzer_on(&buzzer_dev);
      }
      else
      {
        buzzer_off(&buzzer_dev);
      }
      break;

    case R9DS_CTRL_MODE_AUTO:
    default:
      // 自动模式：本模块不执行任何操作，由 auto_ctrl 接管
      break;
  }
}

#pragma section all restore
