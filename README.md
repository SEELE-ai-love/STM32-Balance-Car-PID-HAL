# 🚗 STM32-Balance-Car-PID-HAL

<p align="center">
  <b>两轮自平衡小车 · STM32 HAL 库 + FreeRTOS 多任务版</b><br>
  串级 PID 姿态控制 · 超声波自动控距 · 蓝牙实时调参
</p>

<p align="center">
  <img src="https://img.shields.io/badge/MCU-STM32F103C8T6-0091BD?style=flat-square&logo=arm&logoColor=white" alt="MCU">
  <img src="https://img.shields.io/badge/Library-STM32Cube%20HAL%20FW_F1%20V1.6.0-0ACF83?style=flat-square" alt="HAL">
  <img src="https://img.shields.io/badge/RTOS-FreeRTOS%20V10.5.1-7A1FA0?style=flat-square&logo=freertos&logoColor=white" alt="FreeRTOS">
  <img src="https://img.shields.io/badge/Compiler-Keil%20MDK%20AC5-EB0000?style=flat-square&logo=armkeil&logoColor=white" alt="Keil">
  <img src="https://img.shields.io/badge/Language-C99-4B8BBE?style=flat-square&logo=c&logoColor=white" alt="C99">
</p>

---

## 📖 项目简介

本项目是一台基于 **STM32F103C8T6（Cortex-M3）** 的两轮自平衡小车，从标准库版本完整迁移至 **STM32 HAL 库**，并接入 **FreeRTOS 实时操作系统**，将原先单循环的裸机程序重构为 **5 个独立 RTOS 任务**。

小车以 **MPU6050 六轴传感器** 完成姿态解算，通过 **串级 PID（角度环 → 速度环 → 距离环）** 实现自平衡、定向行驶与 **HC-SR04 超声波自动跟车控距**；支持 **蓝牙串口实时调参** 与 **OLED 实时状态显示**，是一款软硬件全栈的嵌入式综合项目。

---

## ✨ 功能特性

| 功能 | 说明 |
| ---- | ---- |
| ⚖️ 两轮自平衡 | MPU6050 姿态解算（互补滤波）+ 串级 PID，稳态抗扰动 |
| 📡 超声波控距 | HC-SR04 测距，距离环自动调整速度目标，实现 20cm 跟车 |
| 🔵 蓝牙调参 | USART2 透传，上位机在线修改 Kp / Ki / Kd，免重刷 |
| 📺 OLED 显示 | 软件 I2C 驱动，实时显示姿态角、速度、距离与 PID 参数 |
| 🎮 按键控制 | 单键启动 / 停止，交互简单可靠 |
| ⏱️ 多任务调度 | FreeRTOS 5 任务 + 绝对延时，周期恒定互不阻塞 |

---

## 🧠 控制架构（串级 PID + RTOS 多任务）

```
                    ┌──────────────────────────────────────────┐
   目标距离 20cm     │              FreeRTOS 任务调度             │
      │             │                                          │
      ▼             │   ┌──────────────┐  10ms   ┌───────────┐ │
  [距离环PID] ──────┼──▶│ Task_Control │────────▶│ 姿态解算    │ │
      │             │   │  优先级 4     │        │ 角度环PID   │ │
      ▼             │   └──────────────┘        └─────┬─────┘ │
   目标速度          │   ┌──────────────┐  50ms        │       │
      │             │   │  Task_Speed  │◀─────────────┘       │
      ▼             │   │  优先级 3     │                      │
  [速度环PID] ──────┼──▶│  速度+转向    │──▶ PWM → 电机       │
      │             │   └──────────────┘                      │
      ▼             │   ┌──────────────┐  100ms               │
  目标角度(0°)       │   │ Task_Distance │◀── HC-SR04 测距     │
      │             │   │  优先级 2     │                      │
      ▼             │   ├──────────────┤  100ms               │
  [角度环PID] ──────┼──▶│  Task_OLED   │──▶ 状态显示          │
                    │   │  优先级 2     │                      │
      │             │   ├──────────────┤  50ms                │
      ▼             │   │ Task_Bluetooth│◀── USART2 调参       │
   PWM 电机驱动      │   │  优先级 1     │                      │
                    │   └──────────────┘                      │
                    └──────────────────────────────────────────┘
```

### FreeRTOS 任务划分

| 任务 | 周期 | 优先级 | 栈(字) | 职责 |
| ---- | ---- | ---- | ---- | ---- |
| `Task_Control` | 10 ms | 4（最高） | 256 | 姿态解算 + 角度环，自平衡核心 |
| `Task_Speed` | 50 ms | 3 | 256 | 速度环 + 转向环，输出 PWM |
| `Task_Distance` | 100 ms | 2 | 256 | HC-SR04 测距 + 距离环 |
| `Task_OLED` | 100 ms | 2 | 256 | OLED 状态刷新 |
| `Task_Bluetooth` | 50 ms | 1（最低） | 256 | 蓝牙指令解析与 PID 调参 |

> 优先级按 **实时性需求** 理性分配：姿态/角度环最实时最高优先；速度/距离环为外环适当放宽；OLED、蓝牙属锦上添花放最低。所有任务使用 `vTaskDelayUntil` **绝对延时**，周期恒定、不受任务体耗时影响。

---

## 🛠️ 技术栈

| 类别 | 选型 |
| ---- | ---- |
| 主控芯片 | STM32F103C8T6（Cortex-M3 @ 72 MHz，64 KB Flash，20 KB SRAM） |
| 驱动库 | STM32Cube HAL（STM32Cube_FW_F1 V1.6.0） |
| 实时操作系统 | FreeRTOS Kernel V10.5.1（RVDS/ARM_CM3 移植 + heap_4） |
| 姿态传感器 | MPU6050（软件 I2C + 互补滤波） |
| 测距传感器 | HC-SR04 超声波 |
| 编译器 | Keil MDK-ARM AC5（ARMCC，C99，`--no-multibyte-chars`） |
| 编程语言 | C（标准库风格 HAL 驱动封装） |

---

## 📁 目录结构

```
STM32-Balance-Car-PID-HAL/
├── Start/                  CMSIS 核心 + 器件头文件 + 启动文件
│   ├── core_cm3.h          内核头文件
│   ├── stm32f103xb.h       器件头文件
│   ├── startup_stm32f10x_md.s  启动文件（F103 中容量）
│   └── system_stm32f1xx.c/h
├── Library/                HAL 驱动库 + FreeRTOS 内核
│   ├── Inc/ Src/           stm32f1xx_hal_*.c/h（按需裁剪）
│   └── FreeRTOS/           tasks.c queue.c list.c timers.c + portable/ + heap_4.c
├── System/                 Delay.c/h、Timer.c/h（TIM1 1ms 系统时基）
├── User/                   main.c、PID.c/h、FreeRTOSConfig.h、stm32f1xx_it.c/h
├── Hardware/               全部外设驱动（OLED/电机/编码器/MPU6050/蓝牙/超声波/按键/LED/USB）
├── Objects/ Listings/      编译产物（git 忽略）
├── RTOS_demo.c/h           FreeRTOS 知识点教学示例（宏开关控制）
├── Project.uvprojx         Keil uVision5 工程文件
└── README.md
```

---

## 🔌 引脚接线

| 外设 | 引脚 |
| ---- | ---- |
| 电机 PWM（TIM2 CH1/CH2） | PA0 / PA1 |
| 电机方向 | PB12 / PB13 / PB14 / PB15 |
| 编码器 TIM3（左） | PA6 / PA7 |
| 编码器 TIM4（右） | PB6 / PB7 |
| MPU6050（软件 I2C） | SCL=PB10，SDA=PB11 |
| OLED（软件 I2C） | SCL=PB8，SDA=PB9 |
| 蓝牙 USART2 | TX=PA2，RX=PA3，115200 |
| USB 串口 USART1 | TX=PA9，RX=PA10，115200 |
| 按键 | PB1 / PB0 / PA5 / PA4 |
| LED | PC13 |
| 超声波 HC-SR04 | TRIG=PA8，ECHO=PA11（VCC→5V，GND→GND） |
| 系统时钟 | 外部 8 MHz 晶振，PLL×9 = 72 MHz |

---

## 📦 编译与烧录

1. 使用 **Keil uVision5** 打开根目录 `Project.uvprojx`。
2. Options for Target → Device 确认芯片为 `STM32F103C8`。
3. C/C++ 编译宏已配置：`USE_HAL_DRIVER, STM32F103xB`。
4. 点击 **Build**（AC5 编译器 + C99，源码 GBK 编码，已加 `--no-multibyte-chars`）。
5. 使用 ST-Link / 串口一键下载 `Objects` 目录下的 HEX 文件。

> 编译结果：**0 Error / 0 Warning**，Flash 占用约 38 KB / 64 KB，RAM 约 15.8 KB / 20 KB。

---

## 🎓 FreeRTOS 学习模块（RTOS_demo）

工程根目录附带 `RTOS_demo.c/h`，以 5 个独立教学任务覆盖 FreeRTOS 核心知识点，每段代码均有中文注释：

| 任务 | 知识点 |
| ---- | ---- |
| `Demo_Queue` | 队列：任务间安全传数据（生产者/消费者） |
| `Demo_Delay` | 延时与阻塞：`vTaskDelay` / `vTaskDelayUntil` |
| `Demo_Suspend` | 挂起/恢复：`vTaskSuspend` / `vTaskResume` |
| `Demo_Create` | 任务的动态创建与删除 |
| `Demo_Semaphore` | 二值信号量：任务同步 |

在 `User/main.c` 中通过宏 `RTOS_DEMO_ENABLE` 开关（`1` 运行演示 / `0` 关闭，不影响平衡车）。

---

## 🖥️ 相关软件

| 软件 | 用途 |
| ---- | ---- |
| Keil MDK uVision5 | 工程编译 / 调试 / 下载 |
| STM32CubeMX | 芯片选型、时钟与外设配置参考 |
| 串口调试助手 | 蓝牙调参交互、USB 串口监控 |
| Git / GitHub | 版本管理、代码托管 |
| FreeRTOS 官网 | 内核文档与源码参考 |

---

## 📌 版本记录

| 版本 | 说明 |
| ---- | ---- |
| v1.0 | 标准库版：自平衡 + 蓝牙调参 + 超声波控距 |
| v2.0 | 全量迁移 HAL 库，功能保持一致 |
| v3.0 | 接入 FreeRTOS，裸机单循环 → 5 任务多线程架构 |

---

## 👤 About

大一嵌入式期末综合项目，从原理图、标准库移植、HAL 迁移到 RTOS 重构独立完成，用于个人简历展示与学习沉淀。
