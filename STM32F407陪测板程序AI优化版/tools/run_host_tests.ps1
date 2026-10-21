param([string]$Compiler = 'C:/APP/Dev-Cpp/MinGW64/bin/gcc.exe')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Push-Location -LiteralPath $root
try {
    if (!(Test-Path -LiteralPath $Compiler)) { throw 'Provide -Compiler with a native Windows gcc.exe.' }
    $log = [Collections.Generic.List[string]]::new()
    $log.Add('Host logic regression only; no physical peripherals are exercised.')
    $log.Add('Date: ' + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'))
    & $Compiler -std=c99 -Wall -Wextra -Werror -O2 '0_readme/validation/test_app_protocol.c' -o '0_readme/validation/test_app_protocol.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Protocol test compilation failed.' }
    $result = & './0_readme/validation/test_app_protocol.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Protocol regression failed.' }
    foreach ($line in $result) { $log.Add($line); Write-Output $line }
    & $Compiler -std=c99 -Wall -Wextra -Werror '-I0_readme/tests/uart' '-I6_hardware' '0_readme/tests/uart/ring_buffer_test.c' '6_hardware/usart_ring_buff.c' -o '0_readme/validation/ring_buffer_test.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Ring buffer test compilation failed.' }
    $result = & './0_readme/validation/ring_buffer_test.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Ring buffer regression failed.' }
    foreach ($line in $result) { $log.Add($line); Write-Output $line }
    # Production code uses 32-bit MCU addresses; the native mock is 64-bit.
    # Suppress only that expected pointer-width diagnostic in the host model.
    & $Compiler -std=c99 -Wall -Wextra -Werror -Wno-pointer-to-int-cast -O2 '-I0_readme/tests/support' '0_readme/tests/support/support_test.c' -o '0_readme/validation/support_test.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Support test compilation failed.' }
    $result = & './0_readme/validation/support_test.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Flash/data regression failed.' }
    foreach ($line in $result) { $log.Add($line); Write-Output $line }
    [IO.File]::WriteAllLines((Join-Path $root '0_readme/validation/host_tests.log'), $log, [Text.UTF8Encoding]::new($false))
} finally {
    Pop-Location
}
