# NRF52832 蓝牙板程序 AI 优化版

本程序用 NRF52832 被动接收附近 BLE 广播，经 RSSI 和可选特征前缀筛选后，解析 MAC／唯一码和 SN，通过串口发给上位机。它不主动连接蓝牙设备。

**入门建议：先看运行说明，再看流程图，最后从 `main.c` 顺着各模块阅读。**

- [运行说明与命令示例](0_readme/运行说明.md)
- [程序流程图（SVG）](0_readme/程序流程图.svg) · [PNG 图片](0_readme/程序流程图.png)
- [修复记录、验证结果与上板检查](0_readme/修复与验证.md)
- 程序入口：[main.c](1_user_project/main.c)
- Keil 工程：[nrf52832_ble.uvprojx](1_user_project/nrf52832_ble.uvprojx)，应用目标为 `my_borad`。
- 修改前应用代码和工程备份：[baseline_20260911](0_readme/baseline_20260911)，附 SHA-256 清单。

## 验证方式

在项目根目录打开 PowerShell：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run_tests.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check_arm_build.ps1
```

第一条运行主机回归测试，第二条用 ARM GCC 编译并进行 GNU 回归链接。编译器不在默认位置时，可用 `-Compiler` 指定完整路径。这里只对当前进程放开脚本执行，不改变系统执行策略。

**这些检查不等同于 Keil 生产构建和实板测试。** 当前 `6_obj` 中原有 HEX／AXF 是历史产物，没有用本次代码重新生成；不要把它们当作修复后的固件。实际烧录前请在 Keil 中对 `my_borad` 执行 Rebuild，并完成上板检查。
