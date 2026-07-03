# TC264 完全模型组 · 自然选择 INVC — 2026（第21届）

> **作者** 姜云瀚（24计科） · **代码+硬件继承** 王天宇（22电气） · **车模** 百度 I 型 · **MCU** Infineon AURIX TC264

---

## TODO

- [ ] CPU1 移植 FreeRTOS + 双核 IPC 通讯（硬件锁或 FreeRTOS 多核 API）
- [ ] 下一版 MCU 可能？：Infineon CYT4BB7 三核 FreeRTOS，CMake + Ozone 烧录，重新画板
- [ ] `zf_clock.c` 双核时钟适配

## 已知问题

1. **遥控模式直接断电疯车**：SBUS 帧解析在丢帧边界条件下可能未判定离线，需要每次关机时，把`rc.s3`3段开关，拨到除了手动之外的模式。也就是把正面第三个拨杆拉下到底再断电

---

## 1. 简介

本项目基于 **Infineon TC264 双核处理器**，CPU0 运行 FreeRTOS，CPU1 裸机运行。继承自王天宇第 20 届 Car_car 工程，重新整理了目录结构并移植了 FreeRTOS。底层驱动基于逐飞库，应用层重写。

### 21 届智能车组别规则要点

| 项目 | 说明 |
|------|------|
| **赛道** | 物理椭圆跑道 + 百度 AR 叠加（虚拟行人/障碍物/金币等） |
| **电路板** | 嘉立创免费下单，正面覆铜层放置队伍信息，丝网层含文件链接二维码 |
| **MCU** | Infineon AURIX/TRAVEO、STC、NXP RT/MCX Vision、龙芯 — 本车沿用 TC264 |

---

## 2. 项目结构

```
TC264_FreeRTOS_Demo/
├── code/
│   ├── OS/FreeRTOS/              # FreeRTOS 内核
│   └── User/
│       ├── Algorithm/            # PID 控制器
│       ├── App/                  # 入口 (cpu0_main / cpu1_main) + 配置 + 测试
│       ├── Bsp/                  # 板级驱动 (UART / Encoder / PWM / IO)
│       ├── Device/               # 设备抽象 (电机 / 舵机 / R9DS-SBUS)
│       ├── Module/               # 控制逻辑 (r9ds_ctrl 遥控 / auto_ctrl 自动)
│       ├── Protocol/             # 上下位机 UART 协议 (16字节帧)
│       └── Service/              # ISR 中断服务 和 FreeRTOS的配置文件
├── doc/                          # 串口协议 / ADS 手册 / 嘉立创文件
├── libraries/                    # 英飞凌官方库 + 逐飞通用/驱动库
└── Lcf_Tasking_Tricore_Tc.lsl    # 链接器脚本
```

**三层架构**：OS 层（FreeRTOS）→ User 层（应用）→ Libraries 层（底层）。TC264 双核：CPU0 运行 FreeRTOS 任务调度，CPU1 裸机辅助。`zf_clock.c` 仅配置 CPU0 时钟（TODO：双核时钟）。

> `.vscode/` 中配置了 C/C++ 插件头文件路径与 GBK 编码，消除 IDE 误报。

---

## 3. 任务 & 中断调度

### FreeRTOS 任务（CPU0，优先级 0~9 越大越高）

| 任务 | Pri | 触发方式 | 功能 |
|------|:---:|----------|------|
| `led_task` | 2 | 500ms 周期 | LED4 固定闪烁 |
| `key1_task` | 3 | Key A 信号量 | 蜂鸣器 100ms + LED1 翻转 + 通知测试模块，被复用到上位机发送特定消息也执行 |
| `key2_task` | 3 | Key B 信号量 | 蜂鸣器 100ms + LED2 翻转 + 通知测试模块，被复用到上位机发送特定消息也执行 |
| `key3_task` | 3 | Key C 信号量 | 蜂鸣器 100ms + 通知测试模块，被复用到上位机发送特定消息也执行 |
| `uart3_protocol_task` | 4 | UART3 RX 信号量 | 协议解包 → 自动控制更新；解包失败蜂鸣 1000ms |
| `test_task` | 5 | 测试队列 | **TEST_BOARD 开启**：板级测试菜单 → 电机/编码器/舵机测试；**关闭**：空循环 |

### 中断服务（CPU0）

| 中断 | 周期 | 功能 |
|------|:---:|------|
| `cc61_pit_ch1_isr` | **20 ms** | 控制中枢：模式分发 → 遥控/自动 + 电机 PID |
| `cc61_pit_ch0_isr` | **1 ms** | 按键扫描 + R9DS SBUS 帧解析 |
| `uart0_rx_isr` | 事件 | 串口0 接收 → FIFO |
| `uart1_rx_isr` | 事件 | 串口1 接收 → FIFO（SBUS 100K/8E2） |
| `uart2_rx_isr` | 事件 | 串口2 接收 → FIFO |
| `uart3_rx_isr` | 事件 | 串口3 接收 → FIFO + 信号量通知 `uart3_protocol_task` |

> TC264 中断优先级 1~255（越大越高），所有中断均配置为 CPU0 处理，中断内 `interrupt_global_enable(0)` 开启嵌套。

---

## 4. 控制模式

```
 R9DS 离线 (1.5s) ──→ BUG      不能让他在S3 = 1的时候离线，会疯车
 S3=1 (上位)      ──→ MANUAL   遥控器接管
 S3=2 (中位)      ──→ NONE     电机刹停+舵机回中
 S3=3 (下位)      ──→ AUTO     串口协议接管（默认）
```

| 模式 | 油门 | 转向 | 触发源 |
|------|------|------|--------|
| **MANUAL** | CH1 摇杆 → PID 调速 | CH2 摇杆 → 舵机 | S1 切换滑行/刹停，S2 控制蜂鸣器 |
| **NONE** | 立即刹停 (PID 制动到 0) | 舵机回中 (0°) | — |
| **AUTO** | 串口协议 → PID 调速 | 串口协议 → 舵机 | `uart3_protocol_task` → `auto_ctrl_update()` |

---

## 5. R9DS 遥控器（SBUS）

| 参数 | 值 |
|------|-----|
| 接收机 | 乐迪 AT9S R9DS |
| 协议 | Futaba SBUS，25 字节帧，16 通道 × 11-bit |
| 串口 | UART1，TX=P15.0 / RX=P15.1，100K 波特率，9E2 帧格式 |

**通道映射**（摇杆死区 980~1020）：

| 成员 | SBUS 源 | 物理 | 用途 |
|------|---------|------|------|
| `rc.ch1` | CH3 | 左摇杆垂直 | **油门** ±150 cm/s |
| `rc.ch2` | CH1 | 右摇杆水平 | **转向** ±35° |
| `rc.s1` | CH10 | 2段开关 | 滑行(1) / 刹停(2) |
| `rc.s2` | CH9 | 2段开关 | 蜂鸣器 关(1)/开(2) |
| `rc.s3` | CH5 | 3段开关 | 手动(1)/无控(2)/自动(3) |

**离线检测**：1ms ISR 驱动 `offline_tick` 递增，收到有效帧清零；>1500 即 1.5s 判定离线 → 强制回退 AUTO。

---

## 6. 串口协议

16 字节固定帧：`AA 55` + 命令(1B) + 长度(1B) + 数据(9B) + 校验(1B) + `BB 66`。支持命令：`0x01` 设速度、`0x02` 查速度、`0x03` 速度-时间模式、`0x04` 急停、`0x05` 设舵机角度、`0x06` 查舵机角度、`0x07~09` 模拟按键信号量。

> 完整帧格式、校验算法、测试样例详见 `doc/串口协议.md`。

---

## 7. 电路板测试系统

`TEST_BOARD` 宏（`hardware_config.h`）控制编译。开启后系统进入**板级外设验证模式**：自动屏蔽上位机协议和遥控器控制，三个物理按键分别触发电机、编码器、舵机的独立测试序列。

### 7.1 如何启用

`hardware_config.h` 中取消注释 `#define TEST_BOARD`，重新编译烧录即可。验证完成后**注释掉**该宏，重新编译恢复正常运行模式。关闭时所有测试接口退化为空宏，**零开销**。

### 7.2 架构

```
 Key A/B/C 短按
      │
      ▼
 key1/2/3_task (Pri=3)
      │ 信号量触发
      ├─ 蜂鸣器 100ms + LED 翻转（保留原有行为）
      └─ test_key_notify(cmd) → Queue → test_task (Pri=5)
                                           │
                          ┌────────────────┼────────────────┐
                          ▼                ▼                ▼
                     TEST_CMD_MOTOR  TEST_CMD_ENCODER  TEST_CMD_SERVO
                     (电机 PWM 斜坡)  (编码器读数)      (舵机扫描)
```

- **优先级设计**：`test_task` 优先级 5 > `key_task` 的 3，确保测试命令立即响应
- **安全隔离**：`g_test_mode_active=1` → 20ms ISR 跳过自动控制与电机 PID，由测试任务全权接管
- **紧急停止**：测试运行中按**任意按键** → 队列非空唤醒阻塞等待 → `test_emergency_stop()` 立即关闭 PWM + 舵机回中
- **上位机隔离**：测试启动时挂起 `uart3_protocol_task`，防止上位机数据干扰

### 7.3 测试项目详情

#### 测试 A：电机 PWM 斜坡 — Key A

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证电机驱动 PWM 输出线性度、编码器计数跟随性 |
| **测试流程** | 电机正转，占空比 **0% → 30% → 0%**，每步递增/递减 3%，步进间隔 500ms |
| **输出** | 每步通过 UART3 打印 `[TEST-A] step X/10, PWM duty=XXXX, encoder=±XXXXX` |
| **按下 Key A 启动后** | 电机开始缓慢加速旋转，20 步（共约 10 秒）完成斜坡升降 |
| **测试中按任意键** | 立即急停 — PWM=0，电机断电，打印 `EMERGENCY STOP` |

#### 测试 B：编码器读数 — Key B

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证编码器原始计数值正确（方向、量程、噪声） |
| **测试流程** | 100ms 周期连续打印编码器原始值，最长持续 **20 秒** |
| **操作方式** | 按下 Key B 后**手动旋转电机轴**，观察串口输出数值变化是否与旋转方向/圈数一致 |
| **输出** | `[TEST-B] encoder count = ±XXXXX` |
| **按下 Key B 启动后** | 串口开始每 100ms 打印一次编码器值，用户手动转动电机观察读数 |
| **测试中按任意键** | 提前终止，打印 `STOPPED`；不按键则 20 秒超时自动结束 |

#### 测试 C：舵机扫描 — Key C

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证舵机全量程响应（机械安装、角度线性度） |
| **测试流程** | 舵机角度 **-35° → +35° → -35°** 往返扫描，每步 5°，步进间隔 400ms |
| **输出** | 每步打印 `[TEST-C] →/← servo angle = ±XX deg` |
| **按下 Key C 启动后** | 舵机从左极限缓慢扫到右极限再扫回，全程约 11 秒 |
| **测试中按任意键** | 立即急停 — 舵机回中 0°，打印 `STOPPED` |

### 7.4 测试日志示例

```
========================================
[TEST] Board test mode ENABLED
[TEST] Key A: Motor PWM ramp test
[TEST] Key B: Encoder read test
[TEST] Key C: Servo sweep test
[TEST] Press any key during test → EMERGENCY STOP
========================================

[TEST-A] Motor PWM ramp test START
[TEST-A] Ramp UP: 0% → 30%
[TEST-A] ↑ step  1/10, PWM duty= 300, encoder=   45
[TEST-A] ↑ step  2/10, PWM duty= 600, encoder=  132
...
[TEST-A] Motor PWM ramp test PASSED

[TEST-B] Encoder read test START
[TEST-B] Rotate motor shaft manually — press any key to stop
[TEST-B] encoder count = 256
[TEST-B] encoder count = 512
...
[TEST-B] Encoder read test STOPPED

[TEST-C] Servo sweep test START
[TEST-C] Sweep: -35° → +35°
[TEST-C] → servo angle = -35 deg
[TEST-C] → servo angle = -30 deg
...
[TEST-C] Servo sweep test PASSED
```

---

## 8. 开发注意事项

### 引脚禁忌
**Boot 引脚勿用**：P14.2、P14.3、P14.4、P14.5、P14.6、P10.5、P10.6。P20.2 仅输入。

### 工程操作
1. 新增文件后需 **右键工程 → Refresh** 再编译
2. 优化等级：工程属性 → C/C++ Compiler → Optimization（默认关，推荐 O2）
3. 中断嵌套：ISR 内调用 `interrupt_global_enable(0)` 开启
4. **临界资源未加锁**：为追求速度牺牲了安全——串口 3 发送重入会卡死，使用时注意互斥

### 踩坑记录
- **C++ 不可用**：TC264 工具链的异常处理导致类内成员函数异常，改用纯 C
- **流缓冲区不可用**：原因不明，改用逐飞 FIFO 库
- **舵机居中偏**：机械安装公差，软件补偿无效，认了
- **电机伞齿卡顿**：PID 变化率过大导致 — KP 调小、KD 调大
- **野指针/未初始化外设/临界并发**：HardFault 高发区，务必排查

---

> 参考：`doc/串口协议.md` · `doc/ADS使用说明书` · [ADS 汉化教程](https://zhuanlan.zhihu.com/p/509263657) · 逐飞开源库