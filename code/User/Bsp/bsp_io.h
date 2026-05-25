#ifndef __BSP_IO_H__
#define __BSP_IO_H__

#include "FreeRTOS.h"
#include "semphr.h"
#include "zf_common_headfile.h"

void bsp_io_init();

// 前向声明
struct Key;
struct Led;
struct Buzzer;

// 按键设备定义
extern struct Key key_a_dev;
extern struct Key key_b_dev;
extern struct Key key_c_dev;

// LED设备定义
extern struct Led led_1_dev;
extern struct Led led_2_dev;
// extern struct Led led_3_dev; // 此处led3，被舵机pwm占用，不能初始化，也不能作为小灯
extern struct Led led_4_dev;

// 蜂鸣器设备定义
extern struct Buzzer buzzer_dev;

/*==============================================================================
 * 按键结构体
 * @brief 负责按键去抖，使用二值信号量通知按下事件
 * @details 通过1ms定时器扫描实现软件消抖，当按键按下达到消抖阈值时，
 *          从ISR释放信号量，任务可以阻塞等待该信号量来实现按键事件处理
 *============================================================================*/
typedef struct Key
{
  gpio_pin_enum     pin;          // 按键连接的GPIO引脚
  uint8_t           filter_cnt;   // 去抖计数阈值，需连续检测到该次数才确认按下
  uint8_t           filter_state; // 消抖计数器，记录连续按下检测次数
  uint8_t           pressed;      // 按键确认按下标志，防止一次按下多次触发
  SemaphoreHandle_t sem;          // 二值信号量，按键按下时从ISR释放
} Key;

/**
 * @brief 构造函数，初始化按键成员变量
 * @param key 按键结构体指针
 * @param key_pin 按键连接的GPIO引脚
 * @param filter_cnt 去抖计数阈值，需连续检测到该次数才确认为有效按下
 */
void key_init(Key *key, gpio_pin_enum key_pin, uint8_t filter_cnt);

/**
 * @brief 析构函数，释放二值信号量资源
 * @param key 按键结构体指针
 */
void key_deinit(Key *key);

/**
 * @brief 初始化按键GPIO为上拉输入模式，并创建二值信号量
 * @note 必须在使用前调用此函数
 * @param key 按键结构体指针
 */
void key_init_gpio(Key *key);

/**
 * @brief 1ms定时器扫描处理(应在定时器中断中调用)
 * @note 做消抖处理，按下时释放信号量通知任务
 * @param key 按键结构体指针
 */
void key_scan(Key *key);

/**
 * @brief 获取信号量句柄(用于任务中等待按键事件)
 * @param key 按键结构体指针
 * @return SemaphoreHandle_t 二值信号量句柄，用于xSemaphoreTake()
 */
SemaphoreHandle_t key_get_semaphore(Key *key);

/*==============================================================================
 * LED结构体
 * @brief 负责LED灯控制
 * @details 支持高电平有效和低电平有效两种驱动方式，提供开、关、翻转等操作
 *============================================================================*/
typedef struct Led
{
  gpio_pin_enum pin;         // LED连接的GPIO引脚
  bool          active_high; // 高电平有效标记，true表示高电平点亮
} Led;

/**
 * @brief 构造函数，初始化LED成员变量
 * @param led LED结构体指针
 * @param led_pin LED连接的GPIO引脚
 * @param active_high 高电平点亮(true)或低电平点亮(false)
 */
void led_init(Led *led, gpio_pin_enum led_pin, bool active_high);

/**
 * @brief 初始化LED GPIO为推挽输出模式
 * @note 根据active_high设置初始电平，确保LED初始状态正确
 * @param led LED结构体指针
 */
void led_init_gpio(Led *led);

/**
 * @brief 点亮LED
 * @param led LED结构体指针
 */
void led_on(Led *led);

/**
 * @brief 关闭LED
 * @param led LED结构体指针
 */
void led_off(Led *led);

/**
 * @brief 翻转LED状态
 * @note 切换当前输出电平，可用于状态指示
 * @param led LED结构体指针
 */
void led_toggle(Led *led);

/**
 * @brief 设置LED状态
 * @param led LED结构体指针
 * @param state true:点亮 false:关闭
 */
void led_set(Led *led, bool state);

/*==============================================================================
 * 蜂鸣器结构体
 * @brief 负责蜂鸣器控制
 * @details 使用GPIO推挽输出控制蜂鸣器，低电平触发响铃
 *============================================================================*/
typedef struct Buzzer
{
  gpio_pin_enum pin; // 蜂鸣器连接的GPIO引脚
} Buzzer;

/**
 * @brief 构造函数，使用默认蜂鸣器引脚初始化
 * @param buzzer 蜂鸣器结构体指针
 * @param buzzer_pin 蜂鸣器连接的GPIO引脚
 */
void buzzer_init(Buzzer *buzzer, gpio_pin_enum buzzer_pin);

/**
 * @brief 析构函数
 * @param buzzer 蜂鸣器结构体指针
 */
void buzzer_deinit(Buzzer *buzzer);

/**
 * @brief 初始化蜂鸣器 GPIO为推挽输出模式，默认输出低电平（关闭蜂鸣器）
 * @param buzzer 蜂鸣器结构体指针
 */
void buzzer_init_gpio(Buzzer *buzzer);

/**
 * @brief 开启蜂鸣器，输出高电平触发蜂鸣
 * @param buzzer 蜂鸣器结构体指针
 */
void buzzer_on(Buzzer *buzzer);

/**
 * @brief 关闭蜂鸣器，输出低电平
 * @param buzzer 蜂鸣器结构体指针
 */
void buzzer_off(Buzzer *buzzer);

/**
 * @brief 设置蜂鸣器状态
 * @param buzzer 蜂鸣器结构体指针
 * @param state true:开启 false:关闭
 */
void buzzer_set(Buzzer *buzzer, bool state);


#endif // __BSP_IO_H__ 
