# STM32F407 陪测板程序

本程序是上位机与 NRF52832 蓝牙扫描接收板之间的桥梁：上位机设置筛选条件，NRF52832 接收并解析蓝牙广播，STM32 核验数据并上传唯一标识、SN 和 RSSI，同时在 LCD 上显示状态。

## 从哪里开始看

1. [运行逻辑与流程图](0_readme/运行逻辑与流程图.md)：先了解整机在做什么。
2. [main.c](1_user/main.c)：上电初始化、启动 FreeRTOS、异常停机入口。
3. [my_task.c](6_hardware/my_task.c)：PC 命令、NRF 配置、广播处理与结果反馈。
4. [app_protocol.h](6_hardware/app_protocol.h)：不依赖硬件的组帧、字段校验和报文构造。
5. [审查与验证报告](0_readme/审查与验证报告.md)：已修复问题、验证证据及上板检查项目。

## 工程目录

| 路径 | 作用 |
| --- | --- |
| `1_user/` | Keil 工程、程序入口、芯片时钟、中断入口、稳定的链接脚本 |
| `6_hardware/` 根目录 | **当前编译的应用代码和硬件驱动** |
| `3_stm32f407_fwlib/` | ST 标准外设库，保留原版接口 |
| `4_stm32f407_startup/` | 启动汇编、向量表、主栈配置 |
| `8_Free_RTOS/include/`、`FreeRTOS_core/`、`Free_RTOS_portable/` | 当前使用的 FreeRTOS V9 内核与 ARMCC 移植层 |
| `00_eide/` | EIDE 工程配置，与 Keil 使用同一套业务源码 |
| `0_readme/` | 运行说明、审查报告、验证结果和修改前备份 |
| `tools/`、`0_readme/tests/`、`0_readme/validation/` | 检查脚本、主机测试源码与验证产物 |
| `5_obj/`、`1_user/Listings/` | 旧编译产物；不能作为本次修改已通过 Keil 构建的依据 |

`6_hardware` 下的同名子目录（例如 `usart_dma_all/`、`module_test/`）是历史副本，**不在当前目标的编译列表中**。保留它们供查阅；不要把同名文件同时加入工程。`8_Free_RTOS/FreeRTOSv202112.00/` 是附带发行包，不是当前生产内核。

## 按职责定位源码

| 职责 | 文件 |
| --- | --- |
| 启动及任务协调 | `main.c`、`my_task.c/.h` |
| 协议与数据转换 | `app_protocol.h`、`data_processing.c/.h` |
| 串口、DMA、环形缓冲 | `usart_dma_all.c/.h`、`dma.c/.h`、`usart_ring_buff.c/.h` |
| 上电界面与 LCD | `ui_lcd.c/.h`、`lcd.c/.h`、`font.c/.h`、`coolkit_logo_133x87.c` |
| 按键、计时、提示 | `button.c/.h`、`time3.c/.h`、`buzzer.c/.h` |
| 配置保存与 SPI | `w25q32.c/.h`、`spi1.c/.h`、`spi2.c/.h` |
| 电源及串口选择 | `power_ctrl.c/.h`、`uart_select.c/.h` |
| 共享缓冲与延时 | `variable_global.c/.h`、`systick_delay_freerots.c`、`systick_delay_freertos.h` |

## 打开与验证

- Keil：打开 `1_user/coolkit_test_board_stm32f407.uvprojx`，选择 `ble_passive_scanning`，使用 ARMCC 5.06。完整 Rebuild 后检查错误、告警和 `.map` 中的内存占用。
- EIDE：打开 `00_eide/coolkit_test_board_stm32f407.code-workspace`，使用 AC5 工具链。
- 链接脚本统一为 `1_user/stm32f407_memory.sct`，不再依赖清理构建产物时可能被删除的 `5_obj` 内文件。
- 主机检查方式与结果见审查报告。ARM GCC 检查只生成验证用 `.o`，不生成可烧录固件，也不替代 Keil 链接和上板测试。

源文件延续工程原有 GBK 编码，新增纯协议头文件使用 ASCII，说明文档使用 UTF-8。编辑器打开旧源码时选择 GBK，以免中文注释乱码。

## 修改与回退

本次修改前的文件保存在 `0_readme/review_backup/`，其子路径对应原工程路径。备份不参与编译；需要回退某个文件时先保存当前工作，再复制对应原件。不要将整套备份加入 Keil/EIDE 编译列表。
