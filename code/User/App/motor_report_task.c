/**
 * @file motor_report_task.c
 * @author Rh
 * @brief 电机数据周期上报任务 - vofa JustFloat 协议
 * @version 0.1
 * @date 2026-08-20
 *
 * @note 周期性通过 UART3 下发电机运行数据，上位机用 vofa 观察波形调整 PID。
 *       JustFloat 帧格式: N×float(小端4字节) + tail{0x00,0x00,0x80,0x7f}
 *
 * @copyright Copyright (c) 2026
 */

#include "FreeRTOS.h"
#include "task.h"

#include "bsp_uart.h"
#include "device_motor.h"
#include "motor_report_task.h"

#include <math.h>

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 配置宏
 *============================================================================*/
#define MOTOR_REPORT_PERIOD_MS 20 // 上报周期(ms)，与 20ms 控制周期对齐，波形平滑
#define MOTOR_REPORT_CHANNELS 5   // 上报通道数：目标速度*2、实际速度、PWM输出、PID误差

// 波形配置：设定电机目标速度为 正弦波 / 方波，供 vofa 看跟随波形调 PID
#define MOTOR_REPORT_WAVE_AMP 90.0f      // 波形幅值 (cm/s)
#define MOTOR_REPORT_WAVE_PERIOD_MS 2000 // 波形周期 (ms)

// 波形模式：0=方波，1=正弦波
#define MOTOR_REPORT_WAVE_SINE 0

/*==============================================================================
 * vofa JustFloat 帧尾 0x7F800000 (小端 {0x00,0x00,0x80,0x7f})
 *   - JustFloat 以此 4 字节作为帧结束标志，触发一帧解析
 *============================================================================*/
#define VOFA_JUSTFLOAT_TAIL_0 0x00
#define VOFA_JUSTFLOAT_TAIL_1 0x00
#define VOFA_JUSTFLOAT_TAIL_2 0x80
#define VOFA_JUSTFLOAT_TAIL_3 0x7F

/*==============================================================================
 * 任务实现
 *============================================================================*/

/**
 * @brief 电机数据上报任务
 *
 * 每周期计算 正弦波/方波 目标速度并下发到电机，读取 g_motor 运行状态，
 * 组装 vofa JustFloat 帧发送到 UART3。
 * 通道顺序：
 *   ch0 目标速度 target  (cm/s) —— 正弦波/方波
 *   ch1 目标速度 target  (cm/s) —— 打印两次
 *   ch2 实际速度 speed_cm_s    (cm/s)
 *   ch3 PWM输出  output        (占空比 0~10000)
 *   ch4 PID误差  pid.error     (cm/s)
 *
 * @param pvParameters 任务参数（未使用）
 */
void motor_report_task(void *pvParameters)
{
  float  report[MOTOR_REPORT_CHANNELS];
  uint8  frame[MOTOR_REPORT_CHANNELS * 4 + 4]; // N通道float + 4字节帧尾
  float  target;
  uint32 i;
  uint8 *p;

  (void)pvParameters;

  for (;;)
  {
    // 由系统 tick 计算波形时间（ms）
    uint32 t_ms  = (uint32)xTaskGetTickCount();
    float  phase = 2.0f * 3.14159265f * ((float)(t_ms % MOTOR_REPORT_WAVE_PERIOD_MS) / (float)MOTOR_REPORT_WAVE_PERIOD_MS);

#if MOTOR_REPORT_WAVE_SINE
    // 正弦波：A·sin(2π·t/T)
    target = MOTOR_REPORT_WAVE_AMP * sinf(phase);
#else
    // 方波：前半周期 +AMP，后半周期 -AMP
    target = ((t_ms % MOTOR_REPORT_WAVE_PERIOD_MS) < (MOTOR_REPORT_WAVE_PERIOD_MS / 2)) ? MOTOR_REPORT_WAVE_AMP : -MOTOR_REPORT_WAVE_AMP;
#endif

    // 下发目标电机速度（速度取整）；20ms 中断里的 device_motor_update 会按此闭环
    device_motor_set_speed(&g_motor, (int32)target);

    // 采集当前电机数据（target 打印两次）
    report[0] = target;                       // 目标速度
    report[1] = target;                       // 目标速度（打印两次）
    report[2] = g_motor.speed_pid.speed_cm_s; // 实际速度
    report[3] = (float)g_motor.output;        // PWM输出
    report[4] = g_motor.speed_pid.pid.error;  // PID误差

    // 组装 JustFloat 帧: N×float(小端) + tail{0x00,0x00,0x80,0x7f}
    p = &frame[0];
    for (i = 0; i < MOTOR_REPORT_CHANNELS; i++)
    {
      // 小端写入 float 4 字节
      uint8 *b = (uint8 *)&report[i];
#if defined(__BIG_ENDIAN__)
      p[0] = b[3];
      p[1] = b[2];
      p[2] = b[1];
      p[3] = b[0];
#else
      p[0] = b[0];
      p[1] = b[1];
      p[2] = b[2];
      p[3] = b[3];
#endif
      p += 4;
    }

    // 追加 JustFloat 帧尾 0x00 0x00 0x80 0x7f
    p[0] = VOFA_JUSTFLOAT_TAIL_0;
    p[1] = VOFA_JUSTFLOAT_TAIL_1;
    p[2] = VOFA_JUSTFLOAT_TAIL_2;
    p[3] = VOFA_JUSTFLOAT_TAIL_3;

    // 发送到调试串口（mutex 串行化，任务上下文可用）
    bsp_uart_send_buffer(&bsp_uart0, frame, MOTOR_REPORT_CHANNELS * 4 + 4);

    vTaskDelay(pdMS_TO_TICKS(MOTOR_REPORT_PERIOD_MS));
  }
}

#pragma section all restore
