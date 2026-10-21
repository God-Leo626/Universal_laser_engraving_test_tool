# 串口、DMA 与环形队列审查记录

修改范围：实际工程引用的 `6_hardware/usart_ring_buff.c/.h`、`usart_dma_all.c/.h`、`dma.c/.h`。没有修改同名子目录中的历史副本。原始六文件在 `0_readme/review_backup/6_hardware/`，源文件保留 GBK 编码。

## 已修复的问题

|问题|原代码证据与触发方式|处理|
|---|---|---|
|初始化破坏相邻内存（严重）|`ring_buffer_init_for_usart1` 用 `USART1_REC_LEN=2048` 清零实际只有 1024 字节的数组；启动和每次 USART2 恢复都会触发|改为 `sizeof(ringBuf->buffer)`，新增相邻内存哨兵回归测试|
|任务与接收 ISR 丢失长度更新|共享 `length` 原为普通变量，任务 `length--` 可被 ISR `length++` 打断|共享字段使用 `volatile`，每个字节操作用保存/恢复 PRIMASK 的短临界区保护；保持原 USART 中断优先级|
|队列满时 ISR 阻塞打印|原满/空判断调用 `printf`，接收溢出可能在 ISR 中反复等待串口输出|满/空查询不打印；增加 `dropped` 统计供任务检测断流并重新组帧|
|格式化越界与多任务共享发送区竞争|`vsprintf` 没有长度限制；格式化和 DMA 读共享全局 TX 数组之间没有互斥|每端口创建一次 FreeRTOS mutex，锁保护格式化/复制至 DMA 完成；改为 `vsnprintf`，超长整条拒发，防止发送截断协议|
|空发送可能永久等待|原 0 字节也开启 DMA 然后无限等 TC|长度 0 直接返回；二进制允许恰好等于缓存容量，拒绝空指针和超长|
|DMA 状态/异常处理不完整|旧发送标志只在结束后清；故障时无限等待；部分端口初始化使用 `strlen` 或 `sizeof(pointer)` 作为发送长度|发送前清旧标志、每次重装实际地址和长度；增加 DMA 错误检查及无进展轮询上限；初始化用合法占位长度；故障未停流时禁止覆盖 DMA 正在读的数组|
|USART2 重开会抹掉首包|原 `usart2_enable` 先启动接收，再初始化环形队列|先关闭 IDLE/接收、停止 DMA、清队列/旧状态，再开放接收|
|串口硬件错误后继续拼接破损字节流|原 ISR 不区分 ORE、帧错误、噪声、DMA 传输错误|坏接收批次不入队，累计断流事件供上层丢弃半帧；停止 DMA 使用有界轮询|

另修复未启用 UART4 关闭函数把两个 USART 中断枚举按位或后传给 `USART_ITConfig` 的问题，改为逐一关闭，并停止对应 DMA。默认仍然仅启用 USART1、USART2。

## 结构与接口

- `usart_dma_all.c` 管理串口引脚、8N1 参数、中断、USART2 启停。中断仅搬运接收字节及标记错误，不运行协议解析、不调用 FreeRTOS API。
- `dma.c` 用端口配置表统一四个串口的 DMA 参数和发送流程；缓存仍来自 `variable_global.c` 的 `.dma_buf`，保持可被 DMA 访问的 SRAM 放置方式。
- `usart_ring_buff.c` 提供有界 FIFO，保留旧接口。新增 `read_ringbuffer_count_for_usart1(ring, buf, max_len)` 返回实际读取长度；字符串调用者须自行预留并追加 `\0`。
- `get_ringbuffer_dropped_for_usart1` 返回软件溢出字节数加硬件错误事件数，初始化归零。硬件实际丢失字节数未知，因此此值用于检测变化，不能作为精确物理丢字节总量。
- `usart_dma_get_tx_error_count(USART1/USART2)` 可观察拒发、互斥不可用、DMA 错误和无进展超时。为兼容旧代码，原发送函数仍返回 `void`。

## 保持的运行约定

USART1 为 PA9/PA10，DMA2 Stream7/2；USART2 为 PA2/PA3，DMA1 Stream6/5；通道均为 Channel4。原优先级和 8N1 配置保持。发送依旧同步等待 DMA 完成；DMA 完成表示发送缓存可以复用，最后字节仍可能在串口移位寄存器内，保持原返回时机。接收继续使用普通模式 DMA 加 IDLE 中断，应用任务负责分包/粘包和协议帧处理。

## 验证与边界

主机测试运行 `0_readme/tests/uart/run_ring_tests.ps1`，采用真实环形源文件及 Cortex-M 中断掩码桩，已通过初始化哨兵、FIFO 顺序、满容量/溢出、短读界限、100 次回绕、中断交接模拟、嵌套 PRIMASK 恢复和空参数测试。此测试验证内存和算法，不模拟真实 DMA 总线时序。

三个修改后的 C 源文件已通过 ARM GCC C99 语法检查。检查时使用仓库内 FreeRTOS `GCC/ARM_CM4F` 端口头以适配 GCC；正式 Keil 工程仍使用原 `Free_RTOS_portable` 端口。完整工程构建结果由总审查记录汇总。

仍需上板验证：两串口长时间并发收发、连续超过 DMA 缓存长度的无间隔字节流、USART2 关闭/恢复交接、多任务输出、异常断线和 DMA 错误。普通 DMA 满缓存后到 IDLE 重装前存在接收窗口，环形队列也保留原 1024 字节容量；本次提供丢失检测，不声称无限连续输入无丢包。无进展等待上限是 CPU 轮询次数而不是精确定时毫秒，需要以实际波特率和硬件负载确认余量。
