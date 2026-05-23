# TC264 双核 FreeRTOS 开发指南

## 目录

1. [概述](#概述)
2. [TC264 双核架构](#tc264-双核架构)
3. [FreeRTOS 在 TC264 上的移植](#freertos-在-tc264-上的移植)
4. [关键配置文件详解](#关键配置文件详解)
5. [双核运行机制](#双核运行机制)
6. [双核通讯机制](#双核通讯机制)
7. [任务创建与管理](#任务创建与管理)
8. [中断配置](#中断配置)
9. [内存布局](#内存布局)
10. [开发注意事项](#开发注意事项)
11. [常见问题](#常见问题)
12. [移植步骤](#移植步骤)

---

## 概述

本文档详细介绍 TC264 双核处理器上 FreeRTOS 的配置、移植和运行机制。TC264 是英飞凌 AURIX 系列芯片，采用双核 TriCore 架构，本项目基于逐飞科技的开源库和 FreeRTOS V9.0.0 版本进行开发。

### 项目结构

```
E02_uart_demo/
├── code/
│   ├── AppSw/Tricore/OSTasks/
│   │   ├── FreeRTOSConfig.h    # FreeRTOS 配置文件
│   │   └── OsTasks.c           # 任务定义文件
│   └── OS/FreeRTOS/
│       ├── portable/TriCore/
│       │   ├── port.c          # 移植核心文件
│       │   ├── portmacro.h     # 宏定义
│       │   └── port_tricore.h  # TriCore 硬件抽象
│       └── include/            # FreeRTOS 头文件
├── user/
│   ├── cpu0_main.c            # CPU0 入口
│   ├── cpu1_main.c            # CPU1 入口
│   ├── isr.c                  # 中断服务程序
│   └── isr_config.h           # 中断配置
└── libraries/
    └── infineon_libraries/    # iLLD 底层驱动库
```

---

## TC264 双核架构

### 硬件特性

TC264 是一款双核 TriCore 微控制器，具备以下特性：

| 特性 | CPU0 | CPU1 |
|------|------|------|
| 时钟频率 | 200MHz | 200MHz |
| DSPR (数据暂存器) | 72KB | 120KB |
| PSPR (程序暂存器) | 16KB | 32KB |
| CSA 数量 | 128 个 | 128 个 |
| 中断优先级 | 1-255 | 1-255 |

### 地址空间映射

```
内存区域                地址范围              大小
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
DSPR1 (CPU1 数据)       0x60000000-0x6001DFFF   120KB
PSPR1 (CPU1 程序)       0x60100000-0x60107FFF   32KB
DSPR0 (CPU0 数据)       0x70000000-0x70011FFF   72KB
PSPR0 (CPU0 程序)       0x70100000-0x70103FFF   16KB
PFlash0 (程序Flash)     0x80000000-0x800FFFFF   1MB
PFlash1 (程序Flash)     0x80100000-0x8027FFFF   1536KB
```

### TriCore CSA 机制

TriCore 使用 **CSA (Context Save Areas)** 进行中断上下文保存，这是理解 FreeRTOS 移植的关键：

- **每个 CSA 包含 16 个字 (64 字节)**
- **调用一个任务需要 2 个 CSA**：一个 Upper Context 和一个 Lower Context
- **FCX** (Free CSA Pointer): 指向空闲 CSA 链表头
- **PCX** (Previous CSA Pointer): 当前上下文关联的 CSA
- **LCX** (Last CSA Pointer): 空闲 CSA 链表尾

```
┌─────────────────────────────────────────────────────────────┐
│                    CSA 链表结构                              │
├─────────────────────────────────────────────────────────────┤
│  Task 1 ──┬─ Lower CSA ──? Upper CSA ──? Lower CSA ──? NULL│
│           │                                                │
│  Task 2 ──┴─ Lower CSA ──? Upper CSA ──? NULL              │
│           │                                                │
│  Free ────┴─ Lower CSA ──? Lower CSA ──? ... ──? NULL     │
└─────────────────────────────────────────────────────────────┘
```

---

## FreeRTOS 在 TC264 上的移植

### 移植文件清单

| 文件 | 作用 | 位置 |
|------|------|------|
| `port.c` | 上下文切换、堆栈初始化、调度器启动 | `code/OS/FreeRTOS/portable/TriCore/` |
| `portmacro.h` | 端口相关宏定义、临界区管理 | `code/OS/FreeRTOS/portable/TriCore/` |
| `port_tricore.h` | TriCore 寄存器定义、内联函数 | `code/OS/FreeRTOS/portable/TriCore/` |
| `FreeRTOSConfig.h` | 内核配置参数 | `code/AppSw/Tricore/OSTasks/` |

### 编译器支持

本移植支持多种编译器：

```c
// port_tricore.h
#if defined(__HIGHTEC__)      // GNU 编译器
#define TriCore__mfcr(x)      _mfcr(x)
#define TriCore__mtcr(x, v)   _mtcr(x, v)
#define TriCore__disable()     _disable()
#define TriCore__enable()      _enable()

#elif defined(__TASKING__)     // TASKING 编译器
#define TriCore__mfcr(x)      __mfcr(x)
#define TriCore__mtcr(x, v)   __mtcr(x, v)
#define TriCore__disable()    __disable()
#define TriCore__enable()     __enable()

#elif defined(__ghs__)         // Green Hills 编译器
#define TriCore__mfcr(x)      __mfcr(x)
#define TriCore__mtcr(x, v)   __mtcr(x, v)
```

---

## 关键配置文件详解

### FreeRTOSConfig.h

```c
// ==================== 调度器配置 ====================
#define configUSE_PREEMPTION    1    // 启用抢占式调度
#define configUSE_IDLE_HOOK     1    // 启用空闲钩子
#define configCPU_CLOCK_HZ      200000000UL  // CPU频率 200MHz
#define configTICK_RATE_HZ      1000UL       // 系统时钟 1KHz

// ==================== 任务配置 ====================
#define configMAX_PRIORITIES    10    // 最大优先级数量
#define configMINIMAL_STACK_SIZE 256   // 最小栈大小（字）
#define configTOTAL_HEAP_SIZE    (32U * 1024U)  // 堆大小 32KB

// ==================== 中断配置 ====================
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 31
// 中断优先级 > 31 时不受临界区保护，可调用 FreeRTOS API
#define configKERNEL_INTERRUPT_PRIORITY       1
// 内核中断优先级（Tick 中断），必须为 1

// ==================== 软件定时器 ====================
#define configUSE_TIMERS         1
#define configTIMER_TASK_PRIORITY 9
#define configTIMER_QUEUE_LENGTH  5
#define configTIMER_TASK_STACK_DEPTH configMINIMAL_STACK_SIZE

// ==================== AURIX 芯片类型 ====================
#define configAURIX_GENERATION    TRICORE_AURIX_TC2XX

// ==================== 断言定义 ====================
#define configASSERT(x) if((x)==0){portDISABLE_INTERRUPTS();TriCore__debug();vPortLoopForever();}
```

### 关键配置说明

#### 1. 系统时钟频率

```c
#define configCPU_CLOCK_HZ      200000000UL
```
- TC264 CPU0/CPU1 运行在 200MHz
- 用于计算 tick 时钟周期

#### 2. Tick 时钟配置

```c
#define configTICK_RATE_HZ       1000UL
```
- 系统节拍频率 1000Hz = 1ms 一个 tick
- 每次 tick 中断调用 `vPortSystemTickHandler()`

#### 3. 中断优先级

```c
#define configMAX_SYSCALL_INTERRUPT_PRIORITY  31
#define configKERNEL_INTERRUPT_PRIORITY        1
```

**重要规则**：
- 优先级数值越大，优先级越高（与 STM32 相反）
- 中断优先级 > 31：**可调用 FreeRTOS 中断安全 API** (`FROM_ISR` 版本)
- 中断优先级 ≤ 31：**不可调用** FreeRTOS API，否则导致系统崩溃
- 内核中断（Tick）优先级必须为 1（最低）

#### 4. 芯片代际定义

```c
#define TRICORE_AUDO_FUTURE 0x010
#define TRICORE_AUDO_NG     0x020
#define TRICORE_AUDO_MAX    0x030
#define TRICORE_AURIX_TC2XX 0x110    // TC264
#define TRICORE_AURIX_TC3XX 0x120
#define TRICORE_AURIX_TC4XX 0x130
```

#### 使用延时函数

```c
// 相对延迟
vTaskDelay(1000);  // 延迟 1000ms (基于 configTICK_RATE_HZ)

// 绝对延迟
TickType_t xLastWakeTime = xTaskGetTickCount();
while(1)
{
    vTaskDelayUntil(&xLastWakeTime, 1000);
    // 任务将精确每 1000ms 执行一次
}
```

---

## 中断配置

### 中断优先级宏定义

```c
// isr_config.h 关键定义

// ISR_PRIORITY: 1-255, 数值越大优先级越高
// INT_SERVICE: IfxSrc_Tos_cpu0 / IfxSrc_Tos_cpu1 / IfxSrc_Tos_dma

// PIT 中断配置
#define CCU6_0_CH0_INT_SERVICE  IfxSrc_Tos_cpu0
#define CCU6_0_CH0_ISR_PRIORITY 30

// 串口中断配置
#define UART0_TX_INT_PRIO       10
#define UART0_RX_INT_PRIORITY   11

// DMA 中断配置
#define DMA_INT_SERVICE          IfxSrc_Tos_dma   // DMA 中断优先级范围 0-47
#define DMA_INT_PRIO             60               // 实际优先级
```

### 中断服务程序定义

```c
// 使用 IFX_INTERRUPT 宏定义中断
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);    // 开启中断嵌套
    pit_clear_flag(CCU60_CH0);     // 清除中断标志
    
    // 中断处理代码
}

// 宏参数说明：
// 1. 中断函数名
// 2. INT_SERVICE (0=cpu0, 1=cpu1, 2=dma)
// 3. 中断优先级
```

### 中断嵌套

**重要**：TC264 默认进入中断后自动关闭中断，需要手动开启嵌套：

```c
IFX_INTERRUPT(uart1_rx_isr, 0, 14)
{
    interrupt_global_enable(0);  // 开启中断嵌套
    
    // 此时可以响应更高优先级的中断
    camera_uart_handler();
}
```

---

## 内存布局

### 链接脚本关键配置 (Lcf_Tasking_Tricore_Tc.lsl)

```c
// CSA 配置
#define LCF_CSA0_SIZE    8k     // CPU0 的 CSA 大小
#define LCF_CSA1_SIZE    8k     // CPU1 的 CSA 大小

// 堆栈配置
#define LCF_USTACK0_SIZE 2k    // CPU0 用户栈
#define LCF_ISTACK0_SIZE 1k     // CPU0 中断栈
#define LCF_HEAP_SIZE    2k     // 堆大小

// 内存区域
DSPR0: 0x70000000, 72KB   // CPU0 数据 RAM
DSPR1: 0x60000000, 120KB  // CPU1 数据 RAM
PSPR0: 0x70100000, 16KB   // CPU0 程序 RAM
PSPR1: 0x60100000, 32KB   // CPU1 程序 RAM
```

### 内存布局图

```
┌─────────────────────────────────────────────────────────────┐
│                    TC264 内存布局 (CPU0)                     │
├─────────────────────────────────────────────────────────────┤
│  0x70012000 ┌──────────────────┐                             │
│             │    CSA (8KB)     │  上下文保存区               │
│  0x70010000 ├──────────────────┤                             │
│             │  中断栈 (1KB)    │                             │
│  0x7000FC00 ├──────────────────┤                             │
│             │  用户栈 (2KB)    │                             │
│  0x7000F400 ├──────────────────┤                             │
│             │    堆 (2KB)      │  FreeRTOS 内存池           │
│  0x7000EC00 └──────────────────┘                             │
│             │                      │                         │
│  0x70000000 ┌──────────────────┐  │                         │
│             │   DSPR0 (72KB)  │←─┘                         │
└─────────────────────────────────────────────────────────────┘
```

### FreeRTOS 堆内存

```c
// FreeRTOSConfig.h
#define configTOTAL_HEAP_SIZE    (32U * 1024U)  // 32KB

// 堆内存由 FreeRTOS 动态分配
void *pvPortMalloc(size_t xSize);
void vPortFree(void *pv);
```

---

## 开发注意事项

### 1. 中断优先级配置

**关键警告**：
```c
// 所有中断优先级必须设置为不同的值！
// 相同优先级会导致不可预测的行为
#define CCU6_0_CH0_ISR_PRIORITY 30  // ?
#define CCU6_0_CH1_ISR_PRIORITY 31  // ? 不同
// #define CCU6_0_CH1_ISR_PRIORITY 30  // ? 错误！
```

### 2. FreeRTOS API 调用限制

| 调用位置 | 可用 API | 不可用 API |
|----------|----------|------------|
| 任务中 | 所有 API | 无 |
| 中断优先级 > 31 | `*FromISR()` | 普通 API |
| 中断优先级 ≤ 31 | 无 | 任何 API |

### 3. 临界区使用

```c
// 方式1: 任务级临界区
taskENTER_CRITICAL();
// 临界区代码，不能有阻塞操作
taskEXIT_CRITICAL();

// 方式2: 中断级临界区
uint32_t mask = portSET_INTERRUPT_MASK_FROM_ISR();
portCLEAR_INTERRUPT_MASK_FROM_ISR(mask);
```

### 4. 栈大小配置

```c
// 栈大小以"字"为单位（32位）
#define configMINIMAL_STACK_SIZE 256  // 256 字 = 1KB

// 创建任务时指定
xTaskCreate(task_func, "name", 1024, NULL, 1, NULL);
// 1024 字 = 4KB，建议根据任务复杂度调整
```

### 5. CPU1 的 FreeRTOS 考虑

如果需要在 CPU1 上也运行 FreeRTOS：

```c
// 需要单独配置：
// 1. 创建独立的 FreeRTOSConfig.h
// 2. 配置独立的 Tick 中断（STM1）
// 3. 分配独立的堆内存
// 4. 注意跨核通信
```

---

## 双核通讯机制

TC264 双核通讯是实现多核协作的核心，本节详细介绍双核通讯的原理、实现方法和注意事项。

### 双核通讯概述

TC264 的 CPU0 和 CPU1 共享部分外设和内存空间，但拥有独立的 DSPR/PSPR 区域。双核通讯需要解决以下问题：

| 问题 | 描述 | 解决方案 |
|------|------|----------|
| 数据共享 | 两个核需要访问同一份数据 | 共享内存区域 |
| 同步互斥 | 防止数据竞争 | 互斥锁、信号量 |
| 跨核通知 | 一个核需要通知另一个核 | 软件中断、事件标志 |
| 数据一致性 | 确保读写顺序 | 内存屏障、DMA |

### 共享内存通讯

#### 全局地址转换

TriCore 提供地址转换宏，用于将本地地址转换为全局地址：

```c
#include "IfxCpu.h"

// DSPR 地址转换
// 将本地 DSPR 地址 (0xD00xxxxx) 转换为全局地址 (0x7xxxxxxx)
#define IFXCPU_GLB_ADDR_DSPR(cpu, address) ...

// PSPR 地址转换
#define IFXCPU_GLB_ADDR_PSPR(cpu, address) ...
```

#### 共享内存定义

```c
// 定义在共享内存区域的数据结构（推荐放在 CPU1 的 DSPR 中）
#pragma section "cpu1_dsram"

// 共享数据结构（CPU1 DSPR 区域，地址 0x60000000-0x6001DFFF）
typedef struct {
    volatile uint32_t flag;           // 标志位
    volatile int32_t sensor_data;     // 传感器数据
    volatile float control_output;    // 控制输出
    volatile uint8_t command;         // 命令
} SharedData_t;

// 共享缓冲区
#define SHARED_BUFFER_SIZE 256
typedef struct {
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint8_t buffer[SHARED_BUFFER_SIZE];
} SharedBuffer_t;

SharedData_t shared_data;
SharedBuffer_t shared_buffer;

#pragma section all restore
```

#### 跨核访问示例

```c
// CPU0 访问 CPU1 的共享数据
void cpu0_access_shared_data(void)
{
    // 获取 CPU1 的 DSPR 全局地址
    uint32 global_addr = IFXCPU_GLB_ADDR_DSPR(IfxCpu_ResourceCpu_1, &shared_data);
    
    // 或者直接使用已知的全局地址
    // CPU1 的 DSPR 全局地址从 0x60000000 开始
    SharedData_t *p_shared = (SharedData_t *)0x60000000;  // 假设 shared_data 在 CPU1 DSPR 起始位置
    
    // 读取数据（使用 volatile 确保最新值）
    uint32_t data = p_shared->sensor_data;
    
    // 写入数据
    p_shared->command = 1;
}
```

### 互斥机制

#### Spinlock（自旋锁）

iLLD 提供了 `IfxCpu_setSpinLock` 实现自旋锁：

```c
#include "IfxCpu.h"

// 定义自旋锁
IfxCpu_spinLock spinlock = 0;

// 获取锁（带超时）
boolean acquired = IfxCpu_setSpinLock(&spinlock, 0xFFFF);
if (acquired) {
    // 临界区代码
    shared_data.counter++;
    
    // 释放锁
    IfxCpu_resetSpinLock(&spinlock);
}
```


### 跨核信号通知

#### 软件触发中断（推荐）

利用 TC264 的 SCU 模块实现跨核中断：

```c
// isr_config.h 中定义跨核中断
#define CROSS_CORE_INT_SERVICE    IfxSrc_Tos_cpu1    // CPU1 处理 CPU0 的中断
#define CROSS_CORE_INT_PRIORITY    100                // 中断优先级

// CPU0 发送中断到 CPU1
void send_interrupt_to_cpu1(void)
{
    // 触发 CPU1 的软件中断
    // 使用 SRC 寄存器配置
    volatile Ifx_SRC *src = &MODULE_SRC.SCU.SRCR[0];  // 选择合适的 SRCR
    
    // 配置中断服务和优先级
    IfxSrc_init(src, IfxSrc_Tos_cpu1, CROSS_CORE_INT_PRIORITY);
    IfxSrc_enable(src);
    
    // 触发中断
    IfxSrc_setTrigger(src);
}
```

### 双核同步原语

#### 事件同步（IfxCpu）

iLLD 提供了 `IfxCpu_emitEvent` 和 `IfxCpu_waitEvent` 用于多核同步：

```c
#include "IfxCpu.h"

// 同步事件变量（放在共享内存）
IfxCpu_syncEvent sync_event;

// CPU0 等待 CPU1
void cpu0_sync(void)
{
    // 发送事件
    IfxCpu_emitEvent(&sync_event);
    
    // 等待双方同步
    boolean timeout = IfxCpu_waitEvent(&sync_event, 1000);  // 超时 1 秒
    
    if (timeout) {
        // 处理超时
    }
}

// CPU1 等待 CPU0
void cpu1_sync(void)
{
    // 发送事件
    IfxCpu_emitEvent(&sync_event);
    
    // 等待双方同步
    boolean timeout = IfxCpu_waitEvent(&sync_event, 1000);  // 超时 1 秒
    
    if (timeout) {
        // 处理超时
    }
}
```

#### 屏障同步

```c
// 实现屏障同步（多个核全部到达后继续）
typedef struct {
    volatile uint32_t expected_cores;
    volatile uint32_t arrived_cores;
} Barrier_t;

void barrier_wait(Barrier_t *barrier)
{
    uint32_t my_core = IfxCpu_getCoreIndex();
    
    __disable();
    
    barrier->arrived_cores++;
    
    if (barrier->arrived_cores >= barrier->expected_cores) {
        // 最后一个到达，重置计数器
        barrier->arrived_cores = 0;
        __enable();
        // 所有核继续执行
    } else {
        // 不是最后一个，等待其他核
        __enable();
        while (barrier->arrived_cores > 0) {
            __nop();
        }
    }
}
```

### DMA 跨核传输

#### 使用 DMA 进行零拷贝传输

DMA 可用于在不同核的内存之间传输数据：

```c
#include "IfxDma.h"

// DMA 通道配置
IfxDma_DmaChannel channel;

// 初始化 DMA 通道用于跨核传输
void init_dma_channel(void)
{
    IfxDma_Dma dma = &MODULE_DMA;
    channel = IfxDma_Dma_getChannel(dma, 0);
    
    IfxDma_Dma_ChannelConfig config;
    IfxDma_Dma_initChannelConfig(&config, channel);
    
    // 源地址：CPU0 的全局地址
    config.sourceAddress = IFXCPU_GLB_ADDR_DSPR(IfxCpu_ResourceCpu_0, source_buffer);
    // 目标地址：CPU1 的全局地址
    config.destinationAddress = IFXCPU_GLB_ADDR_DSPR(IfxCpu_ResourceCpu_1, dest_buffer);
    
    config.transferCount = BUFFER_SIZE;
    config.moveSize = IfxDma_DmaMoveSize_8bit;
    config.handshaking = IfxDma_DmaChannelHandshaking_none;
    
    IfxDma_Dma_initChannel(channel, &config);
}

// 启动 DMA 传输
void start_cross_core_transfer(void)
{
    IfxDma_Dma_startChannel(channel);
    
    // 等待传输完成
    while (!IfxDma_Dma_getChannelInterruptStatus(channel)) {
        // 可以执行其他任务
    }
    
    IfxDma_Dma_clearChannelInterruptStatus(channel);
}
```

### 通讯模式选择指南

| 场景 | 推荐方案 | 原因 |
|------|----------|------|
| 简单标志位传递 | 全局变量 + Spinlock | 开销最小 |
| 事件通知 | 软件中断 | 实时性好 |
| 大量数据传输 | DMA + 共享内存 | 效率高 |
| 结构化数据交换 | FreeRTOS 队列 | 接口简单 |
| 状态同步 | Mutex + 共享结构 | 安全可靠 |

### 双核通讯注意事项

#### 1. 内存一致性

```c
// 读取共享数据后使用数据屏障
TriCore__dsync();  // 确保数据读取完成

// 写入共享数据后使用存储屏障
TriCore__mem_barrier();  // 确保数据写入完成

// 多核访问共享数据使用 volatile
volatile uint32_t shared_counter;
```

#### 2. 数据对齐

```c
// 共享数据结构必须4字节对齐
typedef struct __attribute__((aligned(4))) {
    uint32_t value1;
    uint32_t value2;
} AlignedStruct_t;

// 避免跨Cache的数据共享
// TC264 L2 Cache 共享，跨核访问可能需要手动管理Cache一致性
```

#### 3. 避免死锁

```c
// 注意锁的获取顺序，避免循环等待
// 错误示例：
//  CPU0: lock(A) -> lock(B)
//  CPU1: lock(B) -> lock(A)  // 可能死锁

// 正确示例：
//  始终按相同顺序获取锁
//  CPU0: lock(A) -> lock(B)
//  CPU1: lock(A) -> lock(B)
```

#### 4. 中断安全

```c
// 跨核中断必须在中断服务程序中处理
// 确保中断优先级配置正确
// 中断服务程序中不能使用阻塞的 FreeRTOS API

// 正确做法：使用 FromISR 版本
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
xQueueSendFromISR(xQueue, &msg, &xHigherPriorityTaskWoken);
portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
```

#### 5. 调试建议

```c
// 添加调试信息
#define DEBUG_PRINT(...) printf(__VA_ARGS__)

// 在关键位置添加状态标志
volatile uint32_t debug_flag = 0;
debug_flag = 1;  // CPU0 到达某点
while (debug_flag != 2);  // 等待 CPU1
debug_flag = 3;  // 继续执行

// 使用 OneEye 等工具监控跨核通讯
```

### 典型应用场景

#### 场景1：传感器数据处理

```
┌─────────────┐     中断      ┌─────────────┐
│   CPU0      │              │   CPU1      │
│  (采集)     │ ───────────? │  (处理)     │
│             │              │             │
│  摄像头数据  │   共享内存    │  图像处理   │
│  采集完成    │ + 信号量     │  控制输出   │
│  释放信号量  │              │  获取信号量  │
└─────────────┘              └─────────────┘
```

```c
// CPU0: 数据采集任务
void采集_task(void *param)
{
    while (1) {
        // 采集摄像头数据
        capture_image();
        
        // 复制到共享缓冲区
        copy_to_shared_buffer();
        
        // 通知 CPU1
        xSemaphoreGive(xDataReadySemaphore);
        
        vTaskDelay(10);
    }
}

// CPU1: 数据处理任务
void处理_task(void *param)
{
    while (1) {
        // 等待 CPU0 数据就绪
        xSemaphoreTake(xDataReadySemaphore, portMAX_DELAY);
        
        // 从共享缓冲区读取数据
        read_from_shared_buffer();
        
        // 处理数据
        process_image();
        
        vTaskDelay(5);
    }
}
```

#### 场景2：实时控制

```
┌─────────────┐     控制命令   ┌─────────────┐
│   CPU0      │              │   CPU1      │
│  (决策)     │ ───────────? │  (执行)     │
│             │              │             │
│  路径规划   │   共享内存    │  电机控制   │
│  速度计算   │ + 事件组     │  PWM 输出   │
└─────────────┘              └─────────────┘
```

---

## 常见问题

### Q1: 系统卡死无响应

可能原因：
- 中断优先级设置冲突
- 栈溢出
- 死锁（两个任务互相等待）

排查方法：
1. 检查 `configASSERT` 是否触发
2. 检查栈大小是否足够
3. 使用调试器查看当前 PC 和寄存器

### Q2: 任务创建失败

可能原因：
- 堆内存不足
- 优先级超出范围

```c
// 排查堆内存
configTOTAL_HEAP_SIZE 太小
// 或栈深度太大
```

### Q3: 中断不响应

可能原因：
- 全局中断未开启
- 中断优先级配置错误
- 中断服务程序未正确关联

```c
// 确保开启全局中断
interrupt_global_enable(0);

// 确保中断优先级配置正确
// 优先级范围应为 1-255 (CPU 中断) 或 0-47 (DMA 中断)
```

---

## 移植步骤

### 步骤 1: 准备文件结构

```
your_project/
├── code/
│   ├── AppSw/Tricore/OSTasks/
│   │   ├── FreeRTOSConfig.h
│   │   └── OsTasks.c
│   └── OS/FreeRTOS/
│       ├── portable/TriCore/
│       │   ├── port.c
│       │   ├── portmacro.h
│       │   └── port_tricore.h
│       └── include/        # FreeRTOS 核心文件
├── user/
│   ├── cpu0_main.c
│   ├── cpu1_main.c
│   ├── isr.c
│   └── isr_config.h
└── libraries/
    └── infineon_libraries/
```

### 步骤 2: 配置 FreeRTOSConfig.h

根据硬件修改关键配置：

```c
// 根据实际 CPU 频率设置
#define configCPU_CLOCK_HZ      200000000UL

// 根据内存大小调整堆
#define configTOTAL_HEAP_SIZE   (32U * 1024U)

// 根据需要设置任务优先级数量
#define configMAX_PRIORITIES    10
```

### 步骤 3: 配置中断

在 `isr_config.h` 中：

```c
// 确保所有中断优先级唯一
#define CCU6_0_CH0_INT_SERVICE IfxSrc_Tos_cpu0
#define CCU6_0_CH0_ISR_PRIORITY 30  // 确保不与其他中断冲突
```

### 步骤 4: 实现任务

在 `OsTasks.c` 中：

```c
// 任务初始化
void OS_Tasks_init(void)
{
    xTaskCreate(your_task, "task", 1024, NULL, 3, NULL);
    OsTasks_setupTimerInterrupt(&MODULE_STM0);
    vTaskStartScheduler();
}

// 任务函数
void your_task(void *param)
{
    while(1)
    {
        // 任务代码
        vTaskDelay(100);
    }
}
```

### 步骤 5: 编译和调试

1. 清理并重新编译项目
2. 下载到芯片
3. 使用调试器单步跟踪
4. 检查任务切换是否正常

---

## 附录：关键寄存器

### TriCore 系统寄存器

| 寄存器 | 地址 | 功能 |
|--------|------|------|
| PSW | 0xFE04 | 程序状态字 |
| ICR | 0xFE2C | 中断控制寄存器 |
| FCX | 0xFE38 | Free CSA 指针 |
| PCXI | 0xFE00 | 上一个 CSA 索引 |
| SYSCON | 0xFE14 | 系统配置 |

### 关键内联函数

```c
TriCore__mfcr(reg)    // 读取系统寄存器
TriCore__mtcr(reg, v) // 写入系统寄存器
TriCore__disable()     // 关闭中断
TriCore__enable()      // 开启中断
TriCore__dsync()       // 数据同步
TriCore__isync()       // 指令同步
TriCore__svlcx()       // 保存 Lower Context
TriCore__rslcx()       // 恢复 Lower Context
```

---

## 参考资料

- [FreeRTOS 官方文档](https://www.freertos.org/)
- [Infineon AURIX TC2xx 用户手册](https://www.infineon.com/)
- [逐飞科技 TC264 开源库文档](https://seekfree.taobao.com/)

---

**文档版本**: v1.0  
**更新时间**: 2024年  
**适用芯片**: TC264D (tc26xb, BGA292 封装)  
**FreeRTOS 版本**: V9.0.0
