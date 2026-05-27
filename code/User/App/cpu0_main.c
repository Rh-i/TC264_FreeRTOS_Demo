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
  }
}

/**
 * @brief key2触发处理 短按
 * @param pvParameters 任务参数
 */
void key2_task(void *pvParameters)
{
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
      auto_ctrl_update();
    }
    else
    {
      // 蜂鸣器响300ms
      buzzer_on(&buzzer_dev);
      vTaskDelay(300);
      buzzer_off(&buzzer_dev);
    }
  }
}

void test_task(void *pvParameters)
{
  (void)pvParameters;
  while (1)
  {
    // 这里可以放一些测试代码，比如打印R9DS通道数据
    printf("R9DS CH0: %d, CH1: %d, CH2: %d, CH3: %d\n", g_r9ds.rc.ch0, g_r9ds.rc.ch1, g_r9ds.rc.ch2, g_r9ds.rc.ch3);
    vTaskDelay(1000);
  }
}


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

  xTaskCreate(uart3_protocol_task, "u3_p", 1024, NULL, 4, NULL); // 优先级越大越高 0~9
  xTaskCreate(test_task, "test_task", 512, NULL, 5, NULL); // 优先级越大越高 0~9

  start_freertos();

  while (1)
  {
    // 空循环
  }
}

#pragma section all restore
