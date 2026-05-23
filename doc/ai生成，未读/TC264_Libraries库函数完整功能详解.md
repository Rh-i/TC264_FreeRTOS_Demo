# TC264 Libraries 库函数完整功能详解

## 📋 目录

- [第一篇：库架构概览](#第一篇库架构概览)
  - [1. 库结构说明](#1-库结构说明)
  - [2. 开发环境配置](#2-开发环境配置)
  - [3. 编码规范](#3-编码规范)

- [第二篇：通用基础库（zf_common）](#第二篇通用基础库zf_common)
  - [1. 时钟管理（zf_common_clock）](#1-时钟管理zf_common_clock)
  - [2. 调试输出（zf_common_debug）](#2-调试输出zf_common_debug)
  - [3. 中断管理（zf_common_interrupt）](#3-中断管理zf_common_interrupt)
  - [4. 数据类型定义（zf_common_typedef）](#4-数据类型定义zf_common_typedef)
  - [5. FIFO缓冲区（zf_common_fifo）](#5-fifo缓冲区zf_common_fifo)
  - [6. 功能函数（zf_common_function）](#6-功能函数zf_common_function)

- [第三篇：外设驱动库（zf_driver）](#第三篇外设驱动库zf_driver)
  - [1. GPIO驱动（zf_driver_gpio）](#1-gpio驱动zf_driver_gpio)
  - [2. UART驱动（zf_driver_uart）](#2-uart驱动zf_driver_uart)
  - [3. SPI驱动（zf_driver_spi）](#3-spi驱动zf_driver_spi)
  - [4. ADC驱动（zf_driver_adc）](#4-adc驱动zf_driver_adc)
  - [5. 定时器驱动（zf_driver_timer）](#5-定时器驱动zf_driver_timer)
  - [6. PWM驱动（zf_driver_pwm）](#6-pwm驱动zf_driver_pwm)
  - [7. DMA驱动（zf_driver_dma）](#7-dma驱动zf_driver_dma)
  - [8. 外部中断（zf_driver_exti）](#8-外部中断zf_driver_exti)
  - [9. 周期中断定时器（zf_driver_pit）](#9-周期中断定时器zf_driver_pit)
  - [10. Flash驱动（zf_driver_flash）](#10-flash驱动zf_driver_flash)
  - [11. 软SPI驱动（zf_driver_soft_spi）](#11-软spi驱动zf_driver_soft_spi)
  - [12. 软I2C驱动（zf_driver_soft_iic）](#12-软i2c驱动zf_driver_soft_iic)
  - [13. 编码器驱动（zf_driver_encoder）](#13-编码器驱动zf_driver_encoder)
  - [14. 延时函数（zf_driver_delay）](#14-延时函数zf_driver_delay)

- [第四篇：设备驱动库（zf_device）](#第四篇设备驱动库zf_device)
  - [1. 蓝牙模块（zf_device_bluetooth_ch9141）](#1-蓝牙模块zf_device_bluetooth_ch9141)
  - [2. 摄像头模块](#2-摄像头模块)
  - [3. 显示屏模块](#3-显示屏模块)
  - [4. IMU传感器](#4-imu传感器)
  - [5. 其他外设](#5-其他外设)

- [第五篇：英飞凌官方库](#第五篇英飞凌官方库)
  - [1. iLLD驱动库](#1-illd驱动库)
  - [2. Service层](#2-service层)
  - [3. 寄存器定义层](#3-寄存器定义层)

---

## 第一篇：库架构概览

### 1. 库结构说明

#### 1.1 整体架构

```
libraries/
├── infineon_libraries/          # 英飞凌官方库（底层）
│   ├── Configurations/          # 芯片配置文件
│   ├── Infra/                   # 基础设施层
│   │   ├── Platform/            # 平台相关（编译器、内存等）
│   │   └── Sfr/TC26B/_Reg/      # 寄存器定义层
│   ├── Service/                 # 服务层
│   │   ├── CpuGeneric/          # 通用CPU服务
│   │   │   ├── If/              # 接口抽象
│   │   │   ├── StdIf/           # 标准接口
│   │   │   ├── SysSe/           # 系统服务
│   │   │   └── _Utilities/      # 工具函数
│   │   └── ...
│   └── iLLD/TC26B/Tricore/      # 低级驱动库（iLLD）
│       ├── Port/                # GPIO底层驱动
│       ├── Asclin/              # UART底层驱动
│       ├── Qspi/                # SPI底层驱动
│       ├── Vadc/                # ADC底层驱动
│       ├── Ccu6/                # 定时器底层驱动
│       └── ... (35+外设驱动)
│
├── zf_common/                   # 通用基础库（中层）
│   ├── zf_common_clock.c/h      # 时钟管理
│   ├── zf_common_debug.c/h      # 调试输出
│   ├── zf_common_interrupt.c/h  # 中断管理
│   ├── zf_common_typedef.h      # 类型定义
│   ├── zf_common_fifo.c/h       # FIFO缓冲区
│   ├── zf_common_function.c/h   # 通用功能函数
│   ├── zf_common_font.c/h       # 字库
│   └── zf_common_interrupt.c/h  # 中断处理
│
├── zf_driver/                   # 外设驱动库（高层）
│   ├── zf_driver_gpio.c/h       # GPIO驱动
│   ├── zf_driver_uart.c/h       # UART驱动
│   ├── zf_driver_spi.c/h        # SPI驱动
│   ├── zf_driver_adc.c/h        # ADC驱动
│   ├── zf_driver_timer.c/h      # 定时器驱动
│   ├── zf_driver_pwm.c/h        # PWM驱动
│   ├── zf_driver_dma.c/h        # DMA驱动
│   ├── zf_driver_exti.c/h       # 外部中断驱动
│   ├── zf_driver_pit.c/h        # 周期中断定时器
│   ├── zf_driver_flash.c/h      # Flash驱动
│   ├── zf_driver_soft_spi.c/h   # 软SPI驱动
│   ├── zf_driver_soft_iic.c/h   # 软I2C驱动
│   ├── zf_driver_encoder.c/h    # 编码器驱动
│   └── zf_driver_delay.c/h      # 延时函数
│
└── zf_device/                   # 设备驱动库（应用层）
    ├── zf_device_bluetooth_ch9141.c/h  # 蓝牙模块
    ├── zf_device_ips114.c/h            # LCD显示屏
    ├── zf_device_oled.c/h              # OLED显示屏
    ├── zf_device_icm20602.c/h          # IMU传感器
    ├── zf_device_camera.c/h            # 摄像头
    └── ... (30+设备驱动)
```

#### 1.2 分层架构说明

```
┌─────────────────────────────────────────────┐
│          应用层（user/）                      │
│   cpu0_main.c, cpu1_main.c, isr.c          │
├─────────────────────────────────────────────┤
│          设备驱动层（zf_device/）             │
│   蓝牙、摄像头、显示屏、传感器等              │
├─────────────────────────────────────────────┤
│          外设驱动层（zf_driver/）             │
│   GPIO、UART、SPI、ADC等抽象接口             │
├─────────────────────────────────────────────┤
│          通用基础层（zf_common/）             │
│   时钟、调试、中断、FIFO等                   │
├─────────────────────────────────────────────┤
│          英飞凌iLLD层（infineon_libraries）   │
│   底层硬件驱动、寄存器操作                    │
├─────────────────────────────────────────────┤
│          寄存器定义层（Sfr/_Reg/）            │
│   寄存器地址、位域定义                        │
└─────────────────────────────────────────────┘
```

### 2. 开发环境配置

#### 2.1 开发工具

- **IDE**: AURIX Development Studio (ADS) v1.8.0+
- **编译器**: HighTec GCC 或 Tasking
- **调试器**: DAS (Debug Access Server)
- **芯片**: Infineon AURIX TC264D

#### 2.2 编译配置

```
项目属性 → C/C++ Build → Settings

包含路径（Include Paths）:
- libraries/infineon_libraries/Configurations
- libraries/infineon_libraries/Infra/Platform/Tricore/Compilers
- libraries/infineon_libraries/iLLD/TC26B/Tricore
- libraries/zf_common
- libraries/zf_driver
- libraries/zf_device
- code/OS/FreeRTOS/include
- code/User/Bsp
```

### 3. 编码规范

#### 3.1 命名规范

```c
// 枚举类型：小写+下划线
typedef enum {
    UART_0,
    UART_1,
} uart_index_enum;

// 函数名：模块名_功能名
void uart_init(uart_index_enum uartn, uint32 baud, ...);
uint8 uart_read_byte(uart_index_enum uartn);

// 宏定义：大写+下划线
#define UART0_TX_INT_PRIO    10
#define GPIO_HIGH            1

// 类型定义：小写+_t
typedef uint32_t uint32;
```

#### 3.2 函数注释规范

```c
//-------------------------------------------------------------------------------------------------------------------
// @brief       函数功能简述
// @param       参数1         参数说明
// @param       参数2         参数说明
// @return      返回值说明
// @example     使用示例
// @note        注意事项
//-------------------------------------------------------------------------------------------------------------------
```

---

## 第二篇：通用基础库（zf_common）

### 1. 时钟管理（zf_common_clock）

#### 1.1 功能概述

负责TC264系统时钟树的初始化配置，包括：
- 外部振荡器配置
- 锁相环（PLL）设置
- CPU时钟分频
- 外设时钟使能

#### 1.2 核心函数

```c
/**
 * @brief       系统时钟初始化
 * @example     clock_init();
 * @note        必须在main函数开头调用
 *              配置CPU运行频率为200MHz
 */
void clock_init(void);
```

#### 1.3 时钟树结构

```
外部晶振 (20MHz)
    ↓
PLL锁相环
    ↓
系统时钟 (200MHz)
    ↓
┌───────┬───────┬───────┐
CPU0    CPU1    外设总线
200MHz  200MHz  100MHz
```

#### 1.4 使用示例

```c
int core0_main(void)
{
    clock_init();     // 第一步：初始化时钟
    debug_init();     // 第二步：初始化调试串口
    
    // 其他初始化...
    
    while(1);
}
```

---

### 2. 调试输出（zf_common_debug）

#### 2.1 功能概述

提供串口调试输出功能，用于打印日志、变量值等调试信息。

#### 2.2 核心函数

```c
/**
 * @brief       调试串口初始化
 * @note        默认使用UART0，波特率115200
 *              引脚：TX=P14_0, RX=P14_1
 */
void debug_init(void);

/**
 * @brief       打印格式化字符串
 * @param       fmt     格式字符串
 * @param       ...     可变参数
 * @example     debug_printf("Value: %d\r\n", value);
 */
void debug_printf(const char *fmt, ...);

/**
 * @brief       打印16进制数据
 * @param       *buffer     数据缓冲区
 * @param       len         数据长度
 * @example     debug_hex(data, 16);
 */
void debug_hex(const uint8 *buffer, uint32 len);
```

#### 2.3 格式化输出

```c
// 整数输出
debug_printf("Decimal: %d\r\n", 123);        // 输出: Decimal: 123
debug_printf("Hex: 0x%04X\r\n", 0xABCD);     // 输出: Hex: 0xABCD

// 浮点数输出
debug_printf("Float: %.2f\r\n", 3.14159);    // 输出: Float: 3.14

// 字符串输出
debug_printf("String: %s\r\n", "Hello");     // 输出: String: Hello

// 多参数
debug_printf("CPU%d: Task=%s, Priority=%d\r\n", 0, "LED", 5);
```

#### 2.4 注意事项

- 调试串口会占用UART0
- 频繁的printf会影响实时性
- 正式发布版本可关闭调试输出

---

### 3. 中断管理（zf_common_interrupt）

#### 3.1 功能概述

提供全局中断使能/禁用接口，用于临界区保护。

#### 3.2 核心函数

```c
/**
 * @brief       使能全局中断
 * @param       primask     中断掩码（通常为0）
 * @example     interrupt_global_enable(0);
 */
void interrupt_global_enable(uint32 primask);

/**
 * @brief       禁用全局中断
 * @return      之前的中断状态（用于恢复）
 * @example     uint32 state = interrupt_global_disable();
 */
uint32 interrupt_global_disable(void);
```

#### 3.3 底层实现

```c
// zf_common_interrupt.c
void interrupt_global_enable(uint32 primask)
{
    if(primask == 0)
    {
        enableInterrupts();  // TriCore汇编指令
    }
}

uint32 interrupt_global_disable(void)
{
    return (uint32)(!disableInterrupts());  // 返回之前的状态
}
```

#### 3.4 多核安全性

**✅ 安全** - 每个CPU有独立的中断控制寄存器

```c
// CPU0禁用中断（不影响CPU1）
uint32 state0 = interrupt_global_disable();

// CPU1中断仍然正常工作
// CPU1可以响应中断...

// CPU0恢复中断
interrupt_global_enable(state0);
```

#### 3.5 使用示例

```c
// 临界区保护
void critical_section(void)
{
    uint32 state = interrupt_global_disable();  // 关中断
    
    // 访问共享资源（不会被中断打断）
    shared_variable++;
    write_hardware_register(value);
    
    interrupt_global_enable(state);             // 恢复中断
}
```

---

### 4. 数据类型定义（zf_common_typedef）

#### 4.1 功能概述

定义统一的数据类型，提高代码可移植性。

#### 4.2 类型定义

```c
// 标准整数类型
typedef signed char         int8;
typedef signed short        int16;
typedef signed int          int32;
typedef signed long long    int64;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

// 布尔类型
typedef enum {
    FALSE = 0,
    TRUE = !FALSE
} boolean;

// 状态类型
typedef enum {
    SUCCESS = 0,
    ERROR = 1
} status_t;
```

#### 4.3 常用宏定义

```c
// 位操作
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define GET_BIT(REG, BIT)     ((REG) & (BIT))

// 数组长度
#define ARRAY_SIZE(arr)       (sizeof(arr) / sizeof((arr)[0]))

// 最大最小值
#define MIN(a, b)             (((a) < (b)) ? (a) : (b))
#define MAX(a, b)             (((a) > (b)) ? (a) : (b))

// 绝对值
#define ABS(x)                (((x) > 0) ? (x) : -(x))

// 限制范围
#define CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
```

---

### 5. FIFO缓冲区（zf_common_fifo）

#### 5.1 功能概述

提供先进先出（FIFO）环形缓冲区实现，常用于串口、SPI等通信的数据缓存。

#### 5.2 核心数据结构

```c
typedef struct {
    uint8 *buffer;          // 缓冲区指针
    uint32 size;            // 缓冲区大小
    uint32 head;            // 写入指针
    uint32 tail;            // 读取指针
    uint32 count;           // 当前数据量
    uint8 data_width;       // 数据宽度（8位/16位/32位）
} fifo_t;
```

#### 5.3 核心函数

```c
/**
 * @brief       FIFO初始化
 * @param       *fifo       FIFO对象指针
 * @param       data_width  数据宽度（FIFO_DATA_8BIT等）
 * @param       *buffer     缓冲区指针
 * @param       size        缓冲区大小
 * @example     fifo_init(&my_fifo, FIFO_DATA_8BIT, buffer, 256);
 */
void fifo_init(fifo_t *fifo, uint8 data_width, uint8 *buffer, uint32 size);

/**
 * @brief       写入数据到FIFO
 * @param       *fifo       FIFO对象指针
 * @param       *data       数据指针
 * @param       len         数据长度
 * @return      实际写入的字节数
 */
uint32 fifo_write_buffer(fifo_t *fifo, const uint8 *data, uint32 len);

/**
 * @brief       从FIFO读取数据
 * @param       *fifo       FIFO对象指针
 * @param       *data       数据指针
 * @param       *len        请求读取的长度（返回实际读取长度）
 * @param       mode        读取模式（FIFO_READ_AND_CLEAN等）
 */
void fifo_read_buffer(fifo_t *fifo, uint8 *data, uint32 *len, uint8 mode);

/**
 * @brief       获取FIFO已使用空间
 * @param       *fifo       FIFO对象指针
 * @return      已使用的字节数
 */
uint32 fifo_used(fifo_t *fifo);

/**
 * @brief       获取FIFO剩余空间
 * @param       *fifo       FIFO对象指针
 * @return      剩余的字节数
 */
uint32 fifo_free(fifo_t *fifo);

/**
 * @brief       清空FIFO
 * @param       *fifo       FIFO对象指针
 */
void fifo_clean(fifo_t *fifo);
```

#### 5.4 使用示例

```c
// 定义FIFO
#define BUFFER_SIZE 256
static uint8 uart_rx_buffer[BUFFER_SIZE];
static fifo_t uart_rx_fifo;

// 初始化
void uart_fifo_init(void)
{
    fifo_init(&uart_rx_fifo, FIFO_DATA_8BIT, uart_rx_buffer, BUFFER_SIZE);
}

// 中断中写入FIFO
void uart_rx_callback(void)
{
    uint8 data;
    uart_query_byte(UART0, &data);
    fifo_write_buffer(&uart_rx_fifo, &data, 1);
}

// 任务中读取FIFO
void uart_process_task(void *pv)
{
    uint8 buffer[64];
    uint32 len;
    
    for(;;)
    {
        if(fifo_used(&uart_rx_fifo) > 0)
        {
            len = 64;
            fifo_read_buffer(&uart_rx_fifo, buffer, &len, FIFO_READ_AND_CLEAN);
            
            // 处理接收到的数据
            process_data(buffer, len);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

#### 5.5 环形缓冲区原理

```
┌───┬───┬───┬───┬───┬───┬───┬───┐
│ 0 │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │  索引
├───┼───┼───┼───┼───┼───┼───┼───┤
│ D │ E │   │   │   │   │ D │ D │  数据
└───┴───┴───┴───┴───┴───┴───┴───┘
        ↑                   ↑
       tail                head

已使用: (head - tail + size) % size = (7 - 2 + 8) % 8 = 5
```

---

### 6. 功能函数（zf_common_function）

#### 6.1 功能概述

提供常用的辅助功能函数，如断言、数据转换等。

#### 6.2 核心函数

```c
/**
 * @brief       断言宏（用于错误检测）
 * @param       condition   条件表达式
 * @example     zf_assert(pointer != NULL);
 * @note        如果条件为假，程序将死机
 */
#define zf_assert(condition)  while(!(condition));

/**
 * @brief       32位数据CRC校验
 * @param       *data       数据指针
 * @param       len         数据长度
 * @return      CRC32值
 */
uint32 crc32_calc(const uint8 *data, uint32 len);

/**
 * @brief       字节序转换（大端<->小端）
 * @param       value       16位值
 * @return      转换后的值
 */
uint16 swap16(uint16 value);

/**
 * @brief       字节序转换（32位）
 * @param       value       32位值
 * @return      转换后的值
 */
uint32 swap32(uint32 value);
```

---

## 第三篇：外设驱动库（zf_driver）

### 1. GPIO驱动（zf_driver_gpio）

#### 1.1 功能概述

提供通用输入输出（GPIO）引脚控制功能，包括：
- 引脚初始化（输入/输出模式）
- 电平读写
- 引脚翻转
- 上下拉配置

#### 1.2 引脚枚举定义

```c
typedef enum {
    // 端口0 (16个引脚)
    P00_0 = 0*32,  P00_1, P00_2,  ..., P00_15,
    
    // 端口2 (16个引脚)
    P02_0 = 2*32,  P02_1, P02_2,  ..., P02_15,
    
    // ... 其他端口
    P10_0 = 10*32, ...,
    P11_0 = 11*32, ...,
    P13_0 = 13*32, ...,
    P14_0 = 14*32, ...,
    P15_0 = 15*32, ...,
    P20_0 = 20*32, ...,
    P21_0 = 21*32, ...,
    P22_0 = 22*32, ...,
    P23_0 = 23*32, ...,
    P32_0 = 32*32, ...,
    P33_0 = 33*32, ...,
} gpio_pin_enum;
```

**⚠️ 特殊引脚说明**：
- `P20_2`: 不能配置为输出模式（仅输入功能）
- `P21_6`: TC264DA芯片无法使用

#### 1.3 模式枚举

```c
typedef enum {
    // 输入模式
    GPI_FLOATING_IN,       // 浮空输入
    GPI_PULL_UP,           // 上拉输入
    GPI_PULL_DOWN,         // 下拉输入
    
    // 输出模式
    GPO_PUSH_PULL,         // 推挽输出
    GPO_OPEN_DTAIN,        // 开漏输出
} gpio_mode_enum;
```

#### 1.4 核心函数详解

##### 1.4.1 GPIO初始化

```c
/**
 * @brief       GPIO引脚初始化
 * @param       pin         引脚编号（如P00_0）
 * @param       dir         方向（GPI输入/GPO输出）
 * @param       dat         初始电平（仅输出模式有效，0=低电平，1=高电平）
 * @param       pinmode     工作模式（推挽/开漏/上拉/下拉等）
 * @example     gpio_init(P00_0, GPO, 1, GPO_PUSH_PULL);
 * @note        初始化P00_0为输出模式，初始输出高电平，推挽模式
 */
void gpio_init(gpio_pin_enum pin, gpio_dir_enum dir, uint8 dat, gpio_mode_enum pinmode);
```

**底层实现**：
```c
void gpio_init(gpio_pin_enum pin, gpio_dir_enum dir, uint8 dat, gpio_mode_enum pinmode)
{
    IfxPort_Mode port_mode;
    
    // 根据方向选择模式
    if(dir == GPI)
    {
        switch(pinmode)
        {
            case GPI_FLOATING_IN: port_mode = IfxPort_Mode_inputNoPullDevice; break;
            case GPI_PULL_DOWN:   port_mode = IfxPort_Mode_inputPullDown; break;
            default:              port_mode = IfxPort_Mode_inputPullUp; break;
        }
    }
    else
    {
        switch(pinmode)
        {
            case GPO_OPEN_DTAIN:  port_mode = IfxPort_Mode_outputOpenDrainGeneral; break;
            default:              port_mode = IfxPort_Mode_outputPushPullGeneral; break;
        }
    }
    
    // 设置引脚模式
    IfxPort_setPinMode(get_port(pin), pin&0x1f, port_mode);
    
    // 设置驱动器
    IfxPort_setPinPadDriver(get_port(pin), pin&0x1f, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    
    // 如果是输出模式，设置初始电平
    if(dir == GPO)
    {
        if(dat)
            IfxPort_setPinHigh(get_port(pin), pin&0x1f);
        else
            IfxPort_setPinLow(get_port(pin), pin&0x1f);
    }
}
```

##### 1.4.2 电平设置

```c
/**
 * @brief       设置GPIO电平
 * @param       pin         引脚编号
 * @param       dat         电平值（0=低，1=高）
 * @example     gpio_set_level(P00_0, 1);  // P00_0输出高电平
 */
void gpio_set_level(gpio_pin_enum pin, uint8 dat);

// 快捷宏定义（更高效）
#define gpio_high(x)    (get_port(x))->OMR.U = 1 << ((x)&0x1f)
#define gpio_low(x)     (get_port(x))->OMR.U = 65536 << ((x)&0x1f)
```

**OMR寄存器原子操作原理**：
```
OMR (Output Modification Register):
- 位[15:0]:   设置为高电平（Set）
- 位[31:16]:  设置为低电平（Reset）

gpio_high(P00_0):
OMR = 0x0001 << 0 = 0x00000001  // 设置P00.0为高

gpio_low(P00_0):
OMR = 0x00010000 << 0 = 0x00010000  // 设置P00.0为低

优点：单次32位写操作，原子性保证，不会被中断打断
```

##### 1.4.3 电平读取

```c
/**
 * @brief       读取GPIO电平
 * @param       pin         引脚编号
 * @return      0=低电平，1=高电平
 * @example     uint8 level = gpio_get_level(P00_0);
 */
uint8 gpio_get_level(gpio_pin_enum pin);
```

##### 1.4.4 引脚翻转

```c
/**
 * @brief       翻转GPIO电平
 * @param       pin         引脚编号
 * @example     gpio_toggle_level(P00_0);  // 高变低，低变高
 */
void gpio_toggle_level(gpio_pin_enum pin);
```

##### 1.4.5 方向设置

```c
/**
 * @brief       动态修改GPIO方向
 * @param       pin         引脚编号
 * @param       dir         新方向（GPI/GPO）
 * @param       pinmode     新模式
 * @example     gpio_set_dir(P00_0, GPI, GPI_PULL_UP);  // 改为输入上拉
 */
void gpio_set_dir(gpio_pin_enum pin, gpio_dir_enum dir, gpio_mode_enum pinmode);
```

#### 1.5 完整使用示例

```c
// 示例1：LED控制（输出）
void led_init(void)
{
    gpio_init(P22_1, GPO, 0, GPO_PUSH_PULL);  // 初始化LED引脚，初始低电平
}

void led_on(void)
{
    gpio_high(P22_1);  // 点亮LED
}

void led_off(void)
{
    gpio_low(P22_1);   // 关闭LED
}

void led_toggle(void)
{
    gpio_toggle_level(P22_1);  // 翻转LED状态
}

// 示例2：按键检测（输入）
void button_init(void)
{
    gpio_init(P02_5, GPI, 0, GPI_PULL_UP);  // 初始化按键引脚，上拉输入
}

uint8 button_pressed(void)
{
    return (gpio_get_level(P02_5) == GPIO_LOW);  // 按下为低电平
}

// 示例3：GPIO在FreeRTOS任务中的应用
void led_blink_task(void *pv)
{
    gpio_init(P22_1, GPO, 0, GPO_PUSH_PULL);
    
    for(;;)
    {
        gpio_toggle_level(P22_1);
        vTaskDelay(pdMS_TO_TICKS(500));  // 延时500ms
    }
}
```

#### 1.6 多核安全性

| 操作 | 安全性 | 说明 |
|------|-------|------|
| 不同引脚操作 | ✅ 安全 | OMR寄存器位操作原子性 |
| 同一引脚操作 | ⚠️ 需协调 | 最终状态取决于最后操作者 |
| 初始化函数 | ⚠️ 条件安全 | 避免两核同时初始化同一引脚 |

---

### 2. UART驱动（zf_driver_uart）

#### 2.1 功能概述

提供通用异步收发传输器（UART）通信功能，支持：
- 4个UART模块（UART0-3）
- 可配置波特率
- 多种引脚复用选项
- FIFO缓冲
- 中断收发

#### 2.2 UART模块定义

```c
typedef enum {
    UART_0,    // ASCLIN0
    UART_1,    // ASCLIN1
    UART_2,    // ASCLIN2
    UART_3,    // ASCLIN3
} uart_index_enum;
```

#### 2.3 引脚复用

每个UART模块有多个引脚可选：

```c
// UART0发送引脚选项
typedef enum {
    UART0_TX_P14_0,     // 默认引脚
    UART0_TX_P14_1,
    UART0_TX_P15_2,
    UART0_TX_P15_3,
} uart_tx_pin_enum;

// UART0接收引脚选项
typedef enum {
    UART0_RX_P14_1,     // 默认引脚
    UART0_RX_P15_3,
} uart_rx_pin_enum;

// UART1/2/3有更多引脚选项（见头文件）
```

#### 2.4 核心函数详解

##### 2.4.1 UART初始化

```c
/**
 * @brief       UART初始化
 * @param       uartn       UART模块编号（UART_0~UART_3）
 * @param       baud        波特率（如115200）
 * @param       tx_pin      发送引脚
 * @param       rx_pin      接收引脚
 * @return      实际配置的波特率
 * @example     uart_init(UART0, 115200, UART0_TX_P14_0, UART0_RX_P14_1);
 * @note        默认配置：8位数据，无校验，1位停止位
 *              关闭收发中断，需手动使能
 */
void uart_init(uart_index_enum uartn, uint32 baud, 
               uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin);
```

**底层实现分析**：
```c
void uart_init(uart_index_enum uartn, uint32 baud, 
               uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin)
{
    boolean interrupt_state = disableInterrupts();  // ⚠️ 仅关闭当前CPU中断
    
    volatile Ifx_ASCLIN *module = IfxAsclin_getAddress((IfxAsclin_Index)uartn);
    
    // 初始化配置结构体（⚠️ 使用全局变量uart_config）
    IfxAsclin_Asc_initModuleConfig(&uart_config, module);
    
    // 设置缓冲区
    uart_set_buffer(uartn);
    
    // 设置中断优先级
    uart_set_interrupt_priority(uartn);
    
    // 配置时钟和波特率
    uart_config.clockSource = IfxAsclin_ClockSource_ascFastClock;
    uart_config.baudrate.prescaler = 4;
    uart_config.baudrate.baudrate = (float32)baud;
    uart_config.baudrate.oversampling = IfxAsclin_OversamplingFactor_8;
    
    // 配置引脚
    IfxAsclin_Asc_Pins pins;
    pins.cts = NULL;
    pins.rts = NULL;
    uart_mux(uartn, tx_pin, rx_pin, (uint32*)&pins.tx, (uint32*)&pins.rx);
    pins.rxMode = IfxPort_InputMode_pullUp;
    pins.txMode = IfxPort_OutputMode_pushPull;
    pins.pinDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    uart_config.pins = &pins;
    
    // 初始化UART模块
    IfxAsclin_Asc_initModule(uart_get_handle(uartn), &uart_config);
    
    // 默认关闭中断
    uart_rx_interrupt(uartn, 0);
    uart_tx_interrupt(uartn, 0);
    
    restoreInterrupts(interrupt_state);
}
```

**⚠️ 多核问题**：
- 使用全局变量`uart_config`，多核并发调用会产生竞态条件
- `disableInterrupts()`只保护当前CPU，不保护另一个CPU
- **解决**：添加互斥锁或确保不同核初始化不同UART

##### 2.4.2 数据发送

```c
/**
 * @brief       发送单字节
 * @param       uartn       UART模块编号
 * @param       dat         要发送的字节
 * @example     uart_write_byte(UART0, 0xA5);
 * @note        阻塞式发送，等待FIFO有空闲
 */
void uart_write_byte(uart_index_enum uartn, const uint8 dat);

/**
 * @brief       发送数据块
 * @param       uartn       UART模块编号
 * @param       *buff       数据缓冲区指针
 * @param       len         数据长度
 * @example     uart_write_buffer(UART0, data, 10);
 */
void uart_write_buffer(uart_index_enum uartn, const uint8 *buff, uint32 len);

/**
 * @brief       发送字符串
 * @param       uartn       UART模块编号
 * @param       *str        字符串指针（以'\0'结尾）
 * @example     uart_write_string(UART0, "Hello TC264\r\n");
 */
void uart_write_string(uart_index_enum uartn, const char *str);
```

**底层实现**：
```c
void uart_write_byte(uart_index_enum uartn, const uint8 dat)
{
    IfxAsclin_Asc* uart_handle = uart_get_handle(uartn);
    
    // 等待TX FIFO为空
    while(IfxAsclin_getTxFifoFillLevel(uart_handle->asclin) != 0);
    
    // 写入数据
    IfxAsclin_write8(uart_handle->asclin, &dat, 1);
}
```

##### 2.4.3 数据接收

```c
/**
 * @brief       接收单字节（阻塞式）
 * @param       uartn       UART模块编号
 * @return      接收到的字节
 * @example     uint8 data = uart_read_byte(UART0);
 * @note        如果没有数据会一直等待
 */
uint8 uart_read_byte(uart_index_enum uartn);

/**
 * @brief       查询式接收（非阻塞）
 * @param       uartn       UART模块编号
 * @param       *dat        数据存放指针
 * @return      1=接收成功，0=无数据
 * @example     
 *              uint8 data;
 *              if(uart_query_byte(UART0, &data))
 *              {
 *                  // 处理接收到的数据
 *              }
 */
uint8 uart_query_byte(uart_index_enum uartn, uint8 *dat);
```

##### 2.4.4 中断控制

```c
/**
 * @brief       使能/禁用发送中断
 * @param       uartn       UART模块编号
 * @param       status      1=使能，0=禁用
 * @example     uart_tx_interrupt(UART0, 1);
 */
void uart_tx_interrupt(uart_index_enum uartn, uint32 status);

/**
 * @brief       使能/禁用接收中断
 * @param       uartn       UART模块编号
 * @param       status      1=使能，0=禁用
 * @example     uart_rx_interrupt(UART0, 1);
 */
void uart_rx_interrupt(uart_index_enum uartn, uint32 status);
```

#### 2.5 使用示例

##### 示例1：轮询方式收发

```c
void uart_polling_example(void)
{
    // 初始化UART0
    uart_init(UART_0, 115200, UART0_TX_P14_0, UART0_RX_P14_1);
    
    // 发送字符串
    uart_write_string(UART_0, "Hello World\r\n");
    
    while(1)
    {
        // 查询接收
        uint8 data;
        if(uart_query_byte(UART_0, &data))
        {
            // 回显接收到的数据
            uart_write_byte(UART_0, data);
        }
    }
}
```

##### 示例2：中断方式收发

```c
// 接收缓冲区
#define RX_BUFFER_SIZE 256
static uint8 rx_buffer[RX_BUFFER_SIZE];
static volatile uint32 rx_index = 0;

// 接收中断回调函数
IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    uint8 data;
    if(uart_query_byte(UART_0, &data))
    {
        rx_buffer[rx_index++] = data;
        if(rx_index >= RX_BUFFER_SIZE)
            rx_index = 0;  // 环形缓冲
    }
}

void uart_interrupt_example(void)
{
    // 初始化UART
    uart_init(UART_0, 115200, UART0_TX_P14_0, UART0_RX_P14_1);
    
    // 使能接收中断
    uart_rx_interrupt(UART_0, 1);
    
    // 主循环处理数据
    while(1)
    {
        if(rx_index > 0)
        {
            // 处理接收数据
            process_rx_data(rx_buffer, rx_index);
            rx_index = 0;
        }
    }
}
```

##### 示例3：FreeRTOS任务中使用

```c
void uart_task(void *pv)
{
    uart_init(UART_0, 115200, UART0_TX_P14_0, UART0_RX_P14_1);
    
    char cmd_buffer[64];
    uint32 cmd_index = 0;
    
    for(;;)
    {
        uint8 data;
        if(uart_query_byte(UART_0, &data))
        {
            if(data == '\n' || data == '\r')
            {
                // 接收到完整命令
                cmd_buffer[cmd_index] = '\0';
                uart_write_string(UART_0, "Received: ");
                uart_write_string(UART_0, cmd_buffer);
                uart_write_string(UART_0, "\r\n> ");
                cmd_index = 0;
            }
            else
            {
                if(cmd_index < 63)
                    cmd_buffer[cmd_index++] = data;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

#### 2.6 多核安全性

| 操作 | 安全性 | 说明 |
|------|-------|------|
| uart_init | ❌ 危险 | 使用全局变量uart_config |
| uart_write_byte（不同UART） | ✅ 安全 | 不同模块独立 |
| uart_write_byte（同一UART） | ⚠️ 需同步 | FIFO操作非原子 |
| uart_read_byte（同一UART） | ⚠️ 需同步 | 可能丢失数据 |

---

由于篇幅限制，以下是其他驱动的简要说明（完整版本请参考源代码注释）：

### 3. SPI驱动（zf_driver_spi）

**功能**：串行外设接口，支持主机模式，高速同步通信

**核心函数**：
```c
void spi_init(spi_index_enum spi_n, spi_mode_enum mode, uint32 clock, 
              spi_pin_enum sck, spi_pin_enum mosi, spi_pin_enum miso, spi_pin_enum cs);
uint8 spi_transfer(spi_index_enum spi_n, uint8 data);
void spi_transfer_buffer(spi_index_enum spi_n, const uint8 *tx_buf, uint8 *rx_buf, uint32 len);
```

**应用**：Flash存储器、显示屏、传感器等

### 4. ADC驱动（zf_driver_adc）

**功能**：模数转换器，12位精度，多通道

**核心函数**：
```c
void adc_init(adc_index_enum adc_n);
uint16 adc_read(adc_index_enum adc_n);
float adc_read_voltage(adc_index_enum adc_n);
```

**应用**：电压检测、传感器读取

### 5-14. 其他驱动（简要）

| 驱动 | 功能 | 核心函数 |
|------|------|---------|
| Timer | 输入捕获/输出比较 | timer_init, timer_get_count |
| PWM | 脉宽调制输出 | pwm_init, pwm_set_duty |
| DMA | 直接内存访问 | dma_init, dma_transfer |
| EXTI | 外部中断 | exti_init |
| PIT | 周期中断 | pit_init |
| Flash | Flash读写 | flash_write, flash_read |
| Soft SPI | 软件模拟SPI | soft_spi_init, soft_spi_transfer |
| Soft I2C | 软件模拟I2C | soft_iic_init, soft_iic_write |
| Encoder | 编码器解码 | encoder_init, encoder_get_count |
| Delay | 延时函数 | system_delay_ms, system_delay_us |

---

## 第四篇：设备驱动库（zf_device）

### 1. 蓝牙模块（CH9141）

**功能**：串口转蓝牙透传

**核心函数**：
```c
uint8 bluetooth_ch9141_init(void);
void bluetooth_ch9141_send_data(uint8 *data, uint32 len);
uint32 bluetooth_ch9141_receive_data(uint8 *data, uint32 max_len);
```

### 2. 摄像头模块

支持多种摄像头：
- MT9V03x（全局快门）
- OV7725（CCD）
- SCC8660（CMOS）

**核心函数**：
```c
uint8 camera_init(void);
uint8* camera_get_image(void);
void camera_set_exposure(uint32 exposure);
```

### 3. 显示屏模块

支持多种显示屏：
- IPS114/IPS200（SPI接口LCD）
- OLED（I2C接口）
- TFT180

**核心函数**：
```c
void lcd_init(void);
void lcd_clear(uint16 color);
void lcd_draw_point(uint16 x, uint16 y, uint16 color);
void lcd_draw_string(uint16 x, uint16 y, const char *str);
void lcd_show_image(uint16 x, uint16 y, uint16 width, uint16 height, const uint8 *image);
```

### 4. IMU传感器

支持多种IMU：
- ICM20602（6轴）
- MPU6050（6轴）
- IMU660RA/IMU963RA（6轴/9轴）

**核心函数**：
```c
uint8 imu_init(void);
void imu_get_gyro(int16 *gx, int16 *gy, int16 *gz);
void imu_get_accel(int16 *ax, int16 *ay, int16 *az);
void imu_get_angle(float *roll, float *pitch, float *yaw);
```

---

## 第五篇：英飞凌官方库

### 1. iLLD驱动库

**位置**：`libraries/infineon_libraries/iLLD/TC26B/Tricore/`

**包含35+个底层驱动**：
- Asclin: UART/LIN/SPI底层
- Ccu6: 捕获比较单元
- Dma: 直接内存访问
- Gpt12: 通用定时器
- MultiCan: CAN总线
- Qspi: 高速SPI
- Vadc: 模数转换器
- ... 等

**特点**：
- 官方维护，稳定可靠
- 直接操作寄存器
- 提供中断服务配置

### 2. Service层

**位置**：`libraries/infineon_libraries/Service/CpuGeneric/`

**功能**：
- StdIf: 标准接口抽象
- SysSe: 系统服务（BSP、时间、数学等）
- If: 外设接口抽象

### 3. 寄存器定义层

**位置**：`libraries/infineon_libraries/Infra/Sfr/TC26B/_Reg/`

**内容**：
- 所有外设寄存器的地址映射
- 寄存器位域定义
- 类型定义

**示例**：
```c
// GPIO寄存器定义
typedef struct _Ifx_P {
    Ifx_P_IO IO[16];          // 输入输出寄存器
    Ifx_P_OMR OMR;            // 输出修改寄存器
    Ifx_P_IOCR0 IOCR0;        // I/O控制寄存器
    // ... 更多寄存器
} Ifx_P;

#define MODULE_P00 (*(Ifx_P*)0xF000A000)  // 端口0基地址
```

---

## 附录：快速参考

### A. 常用函数速查

| 功能 | 函数 | 头文件 |
|------|------|--------|
| GPIO初始化 | gpio_init | zf_driver_gpio.h |
| 设置电平 | gpio_high/low | zf_driver_gpio.h |
| 读取电平 | gpio_get_level | zf_driver_gpio.h |
| UART初始化 | uart_init | zf_driver_uart.h |
| 发送数据 | uart_write_byte | zf_driver_uart.h |
| 接收数据 | uart_read_byte | zf_driver_uart.h |
| SPI初始化 | spi_init | zf_driver_spi.h |
| SPI传输 | spi_transfer | zf_driver_spi.h |
| ADC读取 | adc_read | zf_driver_adc.h |
| PWM输出 | pwm_init | zf_driver_pwm.h |
| 延时 | system_delay_ms | zf_driver_delay.h |

### B. 引脚分配建议

```
CPU0负责：
- UART0 (调试输出)
- GPIO P00, P02, P20
- SPI0
- ADC通道0-7

CPU1负责：
- UART1 (蓝牙通信)
- GPIO P11, P13, P22
- SPI1
- ADC通道8-15
```

---

**文档版本**: v1.0  
**最后更新**: 2026-05-20  
**适用平台**: Infineon TC264D  
**库版本**: Seekfree TC264 OpenSource Library
