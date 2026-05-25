/**
 * @file bsp_io.c
 * @author Rh (qq:750920400)
 * @brief IO设备驱动实现 - 按键、蜂鸣器、LED
 * @version 0.1
 * @date 2026-05-25
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "bsp_io.h"

#pragma section all "cpu0_dsram"
// 按键设备 - P22_0
Key key_a_dev;

// 按键设备 - P23_1
Key key_b_dev;

// 按键设备 - P32_4
Key key_c_dev;

// LED设备 - P20_9
Led led_1_dev;

// LED设备 - P20_8
Led led_2_dev;

// LED设备 - P21_5
// 此处led3，被舵机pwm占用，不能初始化，也不能作为小灯
// Led led_3_dev;

// LED设备 - P21_4
Led led_4_dev;

// 蜂鸣器设备 - P22_1
Buzzer buzzer_dev;

#pragma section all restore


/**
 * @brief 总初始化
 *
 */
void bsp_io_init(void)
{
  // 初始化按键
  key_init(&key_a_dev, P22_0, 7);
  key_init(&key_b_dev, P23_1, 7);
  key_init(&key_c_dev, P32_4, 7);

  // 初始化LED
  led_init(&led_1_dev, P20_9, true);
  led_init(&led_2_dev, P20_8, true);
  // led_init(&led_3_dev, P21_5, true); // 此处led3，被舵机pwm占用，不能初始化，也不能作为小灯
  led_init(&led_4_dev, P21_4, true);

  // 初始化蜂鸣器
  buzzer_init(&buzzer_dev, P22_1);

  // 初始化GPIO
  key_init_gpio(&key_a_dev);
  key_init_gpio(&key_b_dev);
  key_init_gpio(&key_c_dev);
  led_init_gpio(&led_1_dev);
  led_init_gpio(&led_2_dev);
  // led_init_gpio(&led_3_dev); // 此处led3，被舵机pwm占用，不能初始化，也不能作为小灯
  led_init_gpio(&led_4_dev);
  buzzer_init_gpio(&buzzer_dev);
}


/*==============================================================================
 * Key 按键实现
 * @brief 通过1ms定时器扫描实现软件消抖，使用二值信号量进行中断与任务间通信
 *============================================================================*/


/**
 * @brief 初始化按键成员变量
 * @param key 按键结构体指针
 * @param key_pin 按键连接的GPIO引脚
 * @param filter_cnt 去抖计数阈值，需连续检测到该次数才确认为有效按下
 */
void key_init(Key *key, gpio_pin_enum key_pin, uint8_t filter_cnt)
{
  key->pin          = key_pin;
  key->filter_cnt   = filter_cnt;
  key->filter_state = 0;
  key->pressed      = 0;
  key->sem          = NULL;
}

/**
 * @brief 析构函数，释放二值信号量资源
 * @param key 按键结构体指针
 */
void key_deinit(Key *key)
{
  if (key->sem != NULL)
  {
    vSemaphoreDelete(key->sem);
    key->sem = NULL;
  }
}

/**
 * @brief 初始化按键GPIO为上拉输入模式，并创建二值信号量
 * @note 配置为上拉输入，按键按下时引脚电平拉低
 * @param key 按键结构体指针
 */
void key_init_gpio(Key *key)
{
  gpio_init(key->pin, GPI, GPIO_HIGH, GPI_PULL_UP);
  key->sem = xSemaphoreCreateBinary();
}

/**
 * @brief 1ms定时器扫描处理(应在定时器中断中调用)
 * @note 做消抖处理，按下时释放信号量通知任务
 * @param key 按键结构体指针
 */
void key_scan(Key *key)
{
  // 按键按下检测
  if (gpio_get_level(key->pin) == 0)
  {
    key->filter_state++;
    if (key->filter_state >= key->filter_cnt && key->pressed == 0)
    {
      key->pressed                             = 1;
      BaseType_t xHigherPriorityTaskWoken_temp = pdFALSE;
      xSemaphoreGiveFromISR(key->sem, &xHigherPriorityTaskWoken_temp);
    }
  }
  else
  {
    key->filter_state = 0;
    key->pressed      = 0;
  }
}

/**
 * @brief 获取信号量句柄(用于任务中等待按键事件)
 * @param key 按键结构体指针
 * @return SemaphoreHandle_t 二值信号量句柄，用于xSemaphoreTake()
 */
SemaphoreHandle_t key_get_semaphore(Key *key)
{
  return key->sem;
}

/*==============================================================================
 * Led LED实现
 * @brief 负责LED灯控制，支持高电平有效和低电平有效两种驱动方式
 *============================================================================*/

/**
 * @brief 初始化LED成员变量
 * @param led LED结构体指针
 * @param led_pin LED连接的GPIO引脚
 * @param active_high 高电平点亮(true)或低电平点亮(false)
 */
void led_init(Led *led, gpio_pin_enum led_pin, bool active_high)
{
  led->pin         = led_pin;
  led->active_high = active_high;
}

/**
 * @brief 初始化LED GPIO为推挽输出模式
 * @note 根据active_high设置初始电平，确保LED初始状态正确
 * @param led LED结构体指针
 */
void led_init_gpio(Led *led)
{
  gpio_init(led->pin, GPO, led->active_high ? GPIO_HIGH : GPIO_LOW, GPO_PUSH_PULL);
}

/**
 * @brief 点亮LED
 * @note 根据active_high设置对应的输出电平
 * @param led LED结构体指针
 */
void led_on(Led *led)
{
  gpio_set_level(led->pin, led->active_high ? 0 : 1);
}

/**
 * @brief 关闭LED
 * @note 根据active_high设置对应的输出电平
 * @param led LED结构体指针
 */
void led_off(Led *led)
{
  gpio_set_level(led->pin, led->active_high ? 1 : 0);
}

/**
 * @brief 翻转LED状态
 * @note 切换当前输出电平，可用于状态指示
 * @param led LED结构体指针
 */
void led_toggle(Led *led)
{
  gpio_toggle_level(led->pin);
}

/**
 * @brief 设置LED状态
 * @param led LED结构体指针
 * @param state true:点亮 false:关闭
 */
void led_set(Led *led, bool state)
{
  if (state)
  {
    led_on(led);
  }
  else
  {
    led_off(led);
  }
}

/*==============================================================================
 * Buzzer 蜂鸣器实现
 * @brief 负责蜂鸣器控制，使用GPIO推挽输出
 *============================================================================*/

/**
 * @brief 构造函数，使用默认蜂鸣器引脚初始化
 * @param buzzer 蜂鸣器结构体指针
 * @param buzzer_pin 蜂鸣器连接的GPIO引脚
 */
void buzzer_init(Buzzer *buzzer, gpio_pin_enum buzzer_pin)
{
  buzzer->pin = buzzer_pin;
}

/**
 * @brief 析构函数
 * @param buzzer 蜂鸣器结构体指针
 */
void buzzer_deinit(Buzzer *buzzer)
{
  (void)buzzer;
}

/**
 * @brief 初始化蜂鸣器 GPIO为推挽输出模式，默认输出低电平（关闭蜂鸣器）
 * @param buzzer 蜂鸣器结构体指针
 */
void buzzer_init_gpio(Buzzer *buzzer)
{
  gpio_init(buzzer->pin, GPO, GPIO_LOW, GPO_PUSH_PULL);
}

/**
 * @brief 开启蜂鸣器，输出高电平触发蜂鸣
 * @param buzzer 蜂鸣器结构体指针
 */
void buzzer_on(Buzzer *buzzer)
{
  gpio_set_level(buzzer->pin, 1);
}

/**
 * @brief 关闭蜂鸣器，输出低电平
 * @param buzzer 蜂鸣器结构体指针
 */
void buzzer_off(Buzzer *buzzer)
{
  gpio_set_level(buzzer->pin, 0);
}

/**
 * @brief 设置蜂鸣器状态
 * @param buzzer 蜂鸣器结构体指针
 * @param state true:开启 false:关闭
 */
void buzzer_set(Buzzer *buzzer, bool state)
{
  if (state)
  {
    buzzer_on(buzzer);
  }
  else
  {
    buzzer_off(buzzer);
  }
}
