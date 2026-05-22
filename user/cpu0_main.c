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
#include "task.h"

#include "app_cfg.h"
#include "bsp_freertos_cpu0.h"

#pragma section all "cpu0_dsram"

/**
 * @brief LED任务
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
 * @brief key1处理
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
    vTaskDelay(pdMS_TO_TICKS(100));
    buzzer_off(&buzzer_dev);
    
    // 翻转LED1
    led_toggle(&led_1_dev);
  }
}

/**
 * @brief key2处理
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
    vTaskDelay(pdMS_TO_TICKS(100));
    buzzer_off(&buzzer_dev);
    
    // 翻转LED2
    led_toggle(&led_2_dev);
  }
}

/**
 * @brief key3处理
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
    vTaskDelay(pdMS_TO_TICKS(100));
    buzzer_off(&buzzer_dev);
    
    // 翻转LED3
    led_toggle(&led_3_dev);
  }
}

/**
 * @brief CPU0主函数
 */
int core0_main(void)
{
  clock_init(); // 获取时钟频率

  user_init(); // 用户的初始化

  cpu_wait_event_ready(); // 等待所有核心初始化完毕

  xTaskCreate(led_task, "led_task", 64, NULL, 3, NULL);    // 优先级越大越高 0~9
  xTaskCreate(key1_task, "key1_task", 256, NULL, 3, NULL); // 优先级越大越高 0~9
  xTaskCreate(key2_task, "key2_task", 256, NULL, 3, NULL); // 优先级越大越高 0~9
  xTaskCreate(key3_task, "key3_task", 256, NULL, 3, NULL); // 优先级越大越高 0~9
  start_freertos();

  while (1)
  {
    /* 空循环 */
  }
}

#pragma section all restore
