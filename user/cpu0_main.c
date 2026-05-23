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
 *
 * @todo  解包之后需要把对应的数据传入小车的运动控制
 *
 * @param pvParameters
 */
void uart3_protocol_task(void *pvParameters)
{
  (void)pvParameters;

  while (1)
  {
    uart_protocol_poll(&g_uart_protocol);
  }
}


void test_task(void *pvParameters)
{
  (void)pvParameters;

  char msg[64];

  while (1)
  {
    // bsp_pwm_set_duty(&bsp_pwm_motor,1000);

    // 机械中值 0°
    // bsp_pwm_set_duty(&bsp_pwm_servo1,4750);
    // device_servo_set_angle(&g_servo, -35);
    // device_servo_set_angle(&g_servo,35);

    int count = bsp_encoder_get_count(&bsp_encoder_tim2);
    sprintf(msg, "cnt: %d\n", count);
    bsp_uart_send_string(&bsp_uart0,msg);
    // bsp_encoder_clear_count(&bsp_encoder_tim2) ;


    vTaskDelay(1000);
  }
}

/**
 * @brief CPU0主函数
 */
int core0_main(void)
{
  clock_init(); // 获取时钟频率

  cpu_wait_event_ready(); // 等待所有核心初始化完毕

  user_init(); // 用户的初始化


  xTaskCreate(led_task, "led", 64, NULL, 2, NULL);    // 优先级越大越高 0~9
  xTaskCreate(key1_task, "key1", 256, NULL, 3, NULL); // 优先级越大越高 0~9
  xTaskCreate(key2_task, "key2", 256, NULL, 3, NULL); // 优先级越大越高 0~9
  xTaskCreate(key3_task, "key3", 256, NULL, 3, NULL); // 优先级越大越高 0~9

  xTaskCreate(uart3_protocol_task, "uart3_protocol", 512, NULL, 4, NULL); // 优先级越大越高 0~9

  xTaskCreate(test_task, "test_task", 512, NULL, 4, NULL); // 优先级越大越高 0~9


  start_freertos();

  while (1)
  {
    /* 空循环 */
  }
}

#pragma section all restore
