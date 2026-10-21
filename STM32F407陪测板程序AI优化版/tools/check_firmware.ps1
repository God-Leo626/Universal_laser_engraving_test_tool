param(
    [string]$Compiler = '',
    [string]$OutputDirectory = '0_readme/validation/arm_gcc'
)

# Compile application/vendor C sources for Cortex-M4 without changing the Keil project.
# The RVDS context-switch assembler and MDK startup/link step require ARMCC 5.
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $root
if (!$Compiler) {
    $installed = Get-Command arm-none-eabi-gcc -ErrorAction SilentlyContinue
    if ($installed) { $Compiler = $installed.Source }
    else {
        $candidates = Get-ChildItem 'C:/APP/STM32CubeIDE' -Filter arm-none-eabi-gcc.exe -Recurse -ErrorAction SilentlyContinue
        $Compiler = $candidates | Select-Object -First 1 -ExpandProperty FullName
    }
}
if (!$Compiler -or !(Test-Path -LiteralPath $Compiler)) {
    throw 'Provide -Compiler with the path to arm-none-eabi-gcc.exe.'
}
$output = [IO.Path]::GetFullPath((Join-Path $root $OutputDirectory))
if (!$output.StartsWith($root + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'OutputDirectory must be inside this workspace.'
}
New-Item -ItemType Directory -Force -Path $output | Out-Null
[xml]$project = Get-Content -LiteralPath '1_user/coolkit_test_board_stm32f407.uvprojx'
$target = $project.Project.Targets.Target | Select-Object -First 1
$gccPort = '8_Free_RTOS/FreeRTOSv202112.00/FreeRTOSv202112.00/FreeRTOS/Source/portable/GCC/ARM_CM4F'
$arguments = @('-c', '-std=c99', '-mcpu=cortex-m4', '-mthumb', '-mfpu=fpv4-sp-d16',
    '-mfloat-abi=hard', '-DSTM32F40_41xxx', '-DUSE_STDPERIPH_DRIVER',
    '-I1_user', '-I3_stm32f407_fwlib/inc', '-I6_hardware', '-I8_Free_RTOS/include',
    "-I$gccPort", '-finput-charset=GBK', '-Wall', '-Wextra', '-Wformat=2',
    '-Werror=implicit-function-declaration', '-Werror=return-type',
    '-ffunction-sections', '-fdata-sections', '-Os')
$log = [Collections.Generic.List[string]]::new()
$log.Add('Cortex-M4 C object compilation only. NOT a Keil build or flashable firmware.')
$log.Add('GCC ARM_CM4F portmacro.h is used only for this check; production keeps RVDS/ARMCC.')
$log.Add(((& $Compiler --version | Select-Object -First 1) -join ''))
$log.Add('Date: ' + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'))
$failed = 0
$compiled = 0
foreach ($file in $target.Groups.Group.Files.File) {
    $source = '1_user/' + $file.FilePath
    if ([IO.Path]::GetExtension($source) -ne '.c') { continue }
    if ($source -like '*Free_RTOS_portable\port.c') {
        $log.Add('SKIP (ARMCC assembly): ' + $file.FilePath)
        continue
    }
    # This MinGW compiler does not accept the Chinese absolute workspace path.
    # Keep all file arguments relative; validation of the resolved output stays above.
    $obj = Join-Path $OutputDirectory (([IO.Path]::GetFileNameWithoutExtension($source)) + '.o')
    $log.Add('COMPILE ' + $file.FilePath)
    # Native compiler diagnostics must be captured even when PowerShell treats stderr as an error.
    $ErrorActionPreference = 'Continue'
    $diagnostics = & $Compiler @arguments $source -o $obj 2>&1
    $status = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    foreach ($line in $diagnostics) { $log.Add($line.ToString()) }
    if ($status -ne 0) { $failed++; Write-Output ('FAIL ' + $file.FilePath) }
    $compiled++
}
$summary = "Compiled: $compiled; failed: $failed; ARMCC port/startup/link: not checked."
$log.Add($summary)
[IO.File]::WriteAllLines((Join-Path $output 'compile.log'), $log, [Text.UTF8Encoding]::new($false))
Write-Output $summary
if ($failed) { exit 1 }
