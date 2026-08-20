/**
 * @file test.c
 * @author Rh (qq:750920400)
 * @brief 电路板测试模块实现
 * @version 0.1
 * @date 2026-06-07
 *
 * @note 测试内容：
 *       Key A → 电机 PWM 0%→30%→0% 斜坡测试，同步打印编码器值
 *       Key B → 编码器手动旋转读数测试（100ms 周期，最长 20s）
 *       Key C → 舵机 -35°?+35° 往返扫描测试
 *       测试中按任意键 → 紧急停止
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "test.h"

#ifdef TEST_BOARD

#include "zf_common_headfile.h"
#include "task.h"
#include <stdio.h>

#include "app_cfg.h"

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 测试状态枚举（模块内部使用）
 *============================================================================*/
typedef enum
{
  TEST_STATE_IDLE    = 0,
  TEST_STATE_MOTOR   = 1,
  TEST_STATE_ENCODER = 2,
  TEST_STATE_SERVO   = 3,
} TestState;

/*==============================================================================
 * 模块内部静态变量
 *============================================================================*/

// 测试模式激活标志（isr.c 通过 extern 引用）
volatile uint8 g_test_mode_active = 0;

// 测试命令队列
static QueueHandle_t g_test_queue = NULL;

// 当前测试状态
static TestState g_test_state = TEST_STATE_IDLE;

// 上位机协议任务句柄
static TaskHandle_t g_uart3_task_handle = NULL;

/*==============================================================================
 * 私有辅助函数
 *============================================================================*/

/**
 * @brief 通过 UART0 发送测试日志
 */
static void test_log(const char *msg)
{
  bsp_uart_send_string(&bsp_uart3, msg);
}

/**
 * @brief 紧急停止 — 关闭电机 PWM + 舵机回中
 */
static void test_emergency_stop(void)
{
  test_log("[TEST] *** EMERGENCY STOP ***\r\n");
  bsp_pwm_set_duty(&bsp_pwm_motor, 0);
  device_servo_reset(&g_servo);
  gpio_set_level(g_motor.dir_pin, MOTOR_DIR_REVERSE);
  g_test_state = TEST_STATE_IDLE;
}

/*==============================================================================
 * 测试序列实现
 *============================================================================*/

/**
 * @brief 测试 A：电机 PWM 斜坡测试
 * @note  占空比 0%→30%→0%，每步 500ms，同步打印编码器值
 *        任意按键按下 → 紧急停止
 */
static void test_motor_pwm(void)
{
  int16 enc;
  char  buf[80];

  test_log("[TEST-A] Motor PWM ramp test START\r\n");
  test_log("[TEST-A] Ramp UP: 0%% → 30%%\r\n");

  gpio_set_level(g_motor.dir_pin, MOTOR_DIR_FORWARD);

  // 斜坡上升：0 → 3000（30%）
  for (int step = 1; step <= 10; step++)
  {
    uint32 duty = step * 300;
    bsp_pwm_set_duty(&bsp_pwm_motor, duty);
    enc = bsp_encoder_get_count(&bsp_encoder_tim2);

    sprintf(buf, "[TEST-A] ↑ step %2d/10, PWM duty=%4lu, encoder=%5d\r\n", step, duty, enc);
    test_log(buf);

    // 带超时的队列等待 → 可即时响应按键停止
    TestCmd cmd;
    if (xQueueReceive(g_test_queue, &cmd, pdMS_TO_TICKS(500)) == pdTRUE)
    {
      test_emergency_stop();
      return;
    }
  }

  test_log("[TEST-A] Ramp DOWN: 30%% → 0%%\r\n");

  // 斜坡下降：2700 → 0
  for (int step = 10; step >= 1; step--)
  {
    uint32 duty = step * 300;
    bsp_pwm_set_duty(&bsp_pwm_motor, duty);
    enc = bsp_encoder_get_count(&bsp_encoder_tim2);

    sprintf(buf, "[TEST-A] ↓ step %2d/10, PWM duty=%4lu, encoder=%5d\r\n", step, duty, enc);
    test_log(buf);

    TestCmd cmd;
    if (xQueueReceive(g_test_queue, &cmd, pdMS_TO_TICKS(500)) == pdTRUE)
    {
      test_emergency_stop();
      return;
    }
  }

  bsp_pwm_set_duty(&bsp_pwm_motor, 0);
  test_log("[TEST-A] Motor PWM ramp test PASSED\r\n");
}

/**
 * @brief 测试 B：编码器读数测试
 * @note  手动旋转电机轴，100ms 周期打印编码器原始计数值
 *        持续 20 秒或任意按键停止
 */
static void test_encoder(void)
{
  int16 enc;
  char  buf[64];

  test_log("[TEST-B] Encoder read test START\r\n");
  test_log("[TEST-B] Rotate motor shaft manually — press any key to stop\r\n");

  for (int i = 0; i < 200; i++) // 最长 200×100ms = 20s
  {
    enc = bsp_encoder_get_count(&bsp_encoder_tim2);
    sprintf(buf, "[TEST-B] encoder count = %d\r\n", enc);
    test_log(buf);

    TestCmd cmd;
    if (xQueueReceive(g_test_queue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE)
    {
      test_log("[TEST-B] Encoder read test STOPPED\r\n");
      return;
    }
  }

  test_log("[TEST-B] Encoder read test PASSED (20s timeout)\r\n");
}

/**
 * @brief 测试 C：舵机扫描测试
 * @note  舵机角度 -35° → +35° → -35° 往返扫描，每步 400ms
 *        任意按键按下 → 紧急停止 + 舵机回中
 */
static void test_servo_sweep(void)
{
  char buf[64];

  test_log("[TEST-C] Servo sweep test START\r\n");
  test_log("[TEST-C] Sweep: -35° → +35°\r\n");

  // 左 → 右
  for (int angle = -35; angle <= 35; angle += 5)
  {
    device_servo_set_angle(&g_servo, angle);

    sprintf(buf, "[TEST-C] → servo angle = %3d deg\r\n", angle);
    test_log(buf);

    TestCmd cmd;
    if (xQueueReceive(g_test_queue, &cmd, pdMS_TO_TICKS(400)) == pdTRUE)
    {
      device_servo_reset(&g_servo);
      test_log("[TEST-C] Servo sweep test STOPPED\r\n");
      return;
    }
  }

  test_log("[TEST-C] Sweep: +35° → -35°\r\n");

  // 右 → 左
  for (int angle = 30; angle >= -35; angle -= 5)
  {
    device_servo_set_angle(&g_servo, angle);

    sprintf(buf, "[TEST-C] ← servo angle = %3d deg\r\n", angle);
    test_log(buf);

    TestCmd cmd;
    if (xQueueReceive(g_test_queue, &cmd, pdMS_TO_TICKS(400)) == pdTRUE)
    {
      device_servo_reset(&g_servo);
      test_log("[TEST-C] Servo sweep test STOPPED\r\n");
      return;
    }
  }

  device_servo_reset(&g_servo);
  test_log("[TEST-C] Servo sweep test PASSED\r\n");
}

/*==============================================================================
 * 公共接口实现
 *============================================================================*/

/**
 * @brief 测试模块初始化
 */
void test_init(TaskHandle_t uart3_task_handle)
{
  // 保存上位机协议任务句柄
  g_uart3_task_handle = uart3_task_handle;

  // 创建测试命令队列（在调度器启动前创建，确保 key 任务可用）
  g_test_queue = xQueueCreate(4, sizeof(TestCmd));
  configASSERT(g_test_queue != NULL);
}

/**
 * @brief 按键通知测试命令
 */
void test_key_notify(TestCmd cmd)
{
  if (g_test_queue != NULL)
  {
    xQueueSendToBack(g_test_queue, &cmd, 0);
  }
}

/**
 * @brief 测试任务入口
 */
void test_task(void *pvParameters)
{
  (void)pvParameters;

  // 挂起上位机串口协议任务（不收上位机数据）
  if (g_uart3_task_handle != NULL)
  {
    vTaskSuspend(g_uart3_task_handle);
  }

  // 激活测试模式标志（5ms ISR 检查此标志，跳过自动控制）
  g_test_mode_active = 1;

  // 打印测试菜单
  test_log("\r\n========================================\r\n");
  test_log("[TEST] Board test mode ENABLED\r\n");
  test_log("[TEST] Key A: Motor PWM ramp test\r\n");
  test_log("[TEST] Key B: Encoder read test\r\n");
  test_log("[TEST] Key C: Servo sweep test\r\n");
  test_log("[TEST] Press any key during test → EMERGENCY STOP\r\n");
  test_log("========================================\r\n\r\n");

  TestCmd cmd;

  while (1)
  {
    // 阻塞等待测试命令
    if (xQueueReceive(g_test_queue, &cmd, portMAX_DELAY) != pdTRUE)
    {
      continue;
    }

    // 如果当前正在运行测试，任何命令 = 紧急停止
    if (g_test_state != TEST_STATE_IDLE)
    {
      test_emergency_stop();
      continue;
    }

    // 空闲状态 → 根据命令启动对应测试
    switch (cmd)
    {
      case TEST_CMD_MOTOR:
        g_test_state = TEST_STATE_MOTOR;
        test_motor_pwm();
        g_test_state = TEST_STATE_IDLE;
        break;

      case TEST_CMD_ENCODER:
        g_test_state = TEST_STATE_ENCODER;
        test_encoder();
        g_test_state = TEST_STATE_IDLE;
        break;

      case TEST_CMD_SERVO:
        g_test_state = TEST_STATE_SERVO;
        test_servo_sweep();
        g_test_state = TEST_STATE_IDLE;
        break;

      default:
        break;
    }
  }
}

#pragma section all restore

#else // !TEST_BOARD — 空任务

/**
 * @brief 测试任务（空 — TEST_BOARD 未开启）
 */
void test_task(void *pvParameters)
{
  (void)pvParameters;
  while (1)
  {
    vTaskDelay(100);
  }
}

#endif // TEST_BOARD
