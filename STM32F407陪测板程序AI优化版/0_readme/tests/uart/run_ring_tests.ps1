param([string]$Compiler = 'C:/APP/Dev-Cpp/MinGW64/bin/gcc.exe')
$ErrorActionPreference = 'Stop'
$Workspace = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../..'))
$Executable = Join-Path $PSScriptRoot 'ring_buffer_test.exe'
Push-Location $Workspace
try {
    & $Compiler -std=c99 -Wall -Wextra -Werror '-I0_readme/tests/uart' '-I6_hardware' '0_readme/tests/uart/ring_buffer_test.c' '6_hardware/usart_ring_buff.c' -o $Executable
    if ($LASTEXITCODE -ne 0) { throw 'Ring buffer test compilation failed.' }
    & $Executable
    if ($LASTEXITCODE -ne 0) { throw 'Ring buffer test failed.' }
} finally {
    Pop-Location
}
