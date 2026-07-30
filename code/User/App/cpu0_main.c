/**
 * @file cpu0_main.c
 * @author Rh (qq:750920400)
 * @brief cpu0主程序入口
 * @version 0.1
 * @date 2026-05-22
 *
 * @note 需要实现的有:任务的创建与定义
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "zf_common_headfile.h"

#include "FreeRTOS.h"
#include "bsp_freertos_cpu0.h"
#include "task.h"

#include "app_cfg.h"
#include "hardware_config.h"
#include "test.h"

#pragma section all "cpu0_dsram"

/**
 * @brief LED固定闪烁任务
 * @param pvParameters 任务参数
 */
void led_task(void *pvParameters)
{
  (void)pvParameters;
  while (1)
  {
    led_toggle(&led_4_dev);
    vTaskDelay(500);
  }
}

/**
 * @brief key1触发处理 短按
 * @param pvParameters 任务参数
 */
void key1_task(void *pvParameters)
{
  uint8_t buffer[16] = {0xaa, 0x55, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0xBB, 0x66};

  (void)pvParameters;
  while (1)
  {
    // 等待key_a信号量
    xSemaphoreTake(key_get_semaphore(&key_a_dev), portMAX_DELAY);

    // 蜂鸣器响100ms
    buzzer_on(&buzzer_dev);
    vTaskDelay(100);
    buzzer_off(&buzzer_dev);

    // 翻转LED1
    led_toggle(&led_1_dev);

    vTaskDelay(500);

    bsp_uart_send_buffer(&bsp_uart3, buffer, 16);

    // 通知测试模块（TEST_BOARD 关闭时为空宏）
    test_key_notify(TEST_CMD_MOTOR);
  }
}

/**
 * @brief key2触发处理 短按
 * @param pvParameters 任务参数
 */
void key2_task(void *pvParameters)
{
  uint8_t buffer[16] = {0xaa, 0x55, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0xBB, 0x66};

  (void)pvParameters;
  while (1)
  {
    // 等待key_b信号量
    xSemaphoreTake(key_get_semaphore(&key_b_dev), portMAX_DELAY);

    // 蜂鸣器响100ms
    buzzer_on(&buzzer_dev);
    vTaskDelay(100);
    buzzer_off(&buzzer_dev);

    // 翻转LED2
    led_toggle(&led_2_dev);

    vTaskDelay(500);

    bsp_uart_send_buffer(&bsp_uart3, buffer, 16);

    // 通知测试模块（TEST_BOARD 关闭时为空宏）
    test_key_notify(TEST_CMD_ENCODER);
  }
}

/**
 * @brief key3触发处理 短按
 * @param pvParameters 任务参数
 */
void key3_task(void *pvParameters)
{
  (void)pvParameters;


  while (1)
  {
    // 等待key_c信号量
    xSemaphoreTake(key_get_semaphore(&key_c_dev), portMAX_DELAY);

    // 蜂鸣器响100ms
    buzzer_on(&buzzer_dev);
    vTaskDelay(100);
    buzzer_off(&buzzer_dev);

    // 翻转LED3
    // led_toggle(&led_3_dev); // 此处led3，被舵机pwm占用，不能初始化，也不能作为小灯

    // 预留硬件盲盒

    // 通知测试模块（TEST_BOARD 关闭时为空宏）
    test_key_notify(TEST_CMD_SERVO);
  }
}

/**
 * @brief 上下位机通讯串口接收处理任务，串口3
 * @note  接收的是中断接收存入fifo的数据，进行解包处理
 *        解包失败响300ms的蜂鸣器
 *
 * @param pvParameters
 */
void uart3_protocol_task(void *pvParameters)
{
  (void)pvParameters;

  while (1)
  {
    if (uart_protocol_poll(&g_uart_protocol))
    {
      // 如果解包到了数据，那我处理。这个不会重复解包
      // 非自动模式时不允许自动控制覆盖遥控器
      if (r9ds_ctrl_get_mode() == R9DS_CTRL_MODE_AUTO)
      {
        auto_ctrl_update();
      }
    }
  }
}


/**
 * @brief 舵机PWM调试任务 — 自动模式下每秒打印舵机占空比
 * @param pvParameters 任务参数
 */
// void debug_servo_duty_task(void *pvParameters)
//{
//   (void)pvParameters;
//   while (1)
//   {
//     // if (r9ds_ctrl_get_mode() == R9DS_CTRL_MODE_AUTO)
//     // {
//       uint32 duty = bsp_pwm_get_duty(g_servo.pwm);
//       printf("[SERVO] AUTO mode, duty=%lu\r\n", duty);
//     // }
//     vTaskDelay(1000);
//   }
// }

/**
 * @brief CPU0主函数
 */
int core0_main(void)
{
  clock_init();           // 获取时钟频率
  cpu_wait_event_ready(); // 等待所有核心初始化完毕
  user_init();            // 用户的初始化

  xTaskCreate(led_task, "led", 64, NULL, 2, NULL);    // 优先级越大越高 0~9
  xTaskCreate(key1_task, "key1", 256, NULL, 3, NULL); // 优先级越大越高 0~9
  xTaskCreate(key2_task, "key2", 256, NULL, 3, NULL); // 优先级越大越高 0~9
  xTaskCreate(key3_task, "key3", 256, NULL, 3, NULL); // 优先级越大越高 0~9

  TaskHandle_t uart3_handle = NULL;
  xTaskCreate(uart3_protocol_task, "u3_p", 2048, NULL, 4, &uart3_handle);

#ifdef TEST_BOARD
  test_init(uart3_handle);                                 // 创建测试队列（需在调度器启动前）
  xTaskCreate(test_task, "test_task", 512, NULL, 5, NULL); // 优先级越大越高 0~9
#endif
  //  xTaskCreate(debug_servo_duty_task, "dbg_svo", 256, NULL, 5, NULL); // 舵机PWM调试打印

  start_freertos();

  while (1)
  {
    // 空循环
  }
}

#pragma section all restore
