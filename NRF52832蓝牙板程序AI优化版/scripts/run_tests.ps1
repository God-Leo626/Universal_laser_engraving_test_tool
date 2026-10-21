param([string]$Compiler = 'gcc')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
New-Item -ItemType Directory -Force -Path 'tests/build' | Out-Null
$flags = @('-std=c99', '-D__USE_MINGW_ANSI_STDIO=1', '-Wall', '-Wextra', '-Werror', '-O2', '-I8_hardware')
& $Compiler @flags 'tests/test_core.c' '8_hardware/data_processing.c' '8_hardware/adv_parser.c' '8_hardware/app_config.c' '8_hardware/uart_commands.c' '-o' 'tests/build/test_core.exe'
if ($LASTEXITCODE -ne 0) { throw 'Core test compilation failed' }
& './tests/build/test_core.exe'
if ($LASTEXITCODE -ne 0) { throw 'Core regression failed' }
& $Compiler @flags '-Itests/stubs' 'tests/test_uart_driver.c' '8_hardware/uarte_52832_dma.c' '8_hardware/my_printf.c' '-o' 'tests/build/test_uart_driver.exe'
if ($LASTEXITCODE -ne 0) { throw 'UART event test compilation failed' }
& './tests/build/test_uart_driver.exe'
if ($LASTEXITCODE -ne 0) { throw 'UART event regression failed' }
& $Compiler @flags '-Itests/stubs' 'tests/test_ble.c' '8_hardware/ble_app.c' '8_hardware/adv_parser.c' '8_hardware/data_processing.c' '8_hardware/app_config.c' '-o' 'tests/build/test_ble.exe'
if ($LASTEXITCODE -ne 0) { throw 'BLE test compilation failed' }
& './tests/build/test_ble.exe'
if ($LASTEXITCODE -ne 0) { throw 'BLE regression failed' }
