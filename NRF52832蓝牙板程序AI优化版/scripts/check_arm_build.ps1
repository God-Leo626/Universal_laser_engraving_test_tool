param([string]$Compiler = '')

# Compile the actual Keil application source list for Cortex-M4 using local GCC.
# This is a cross-compiler regression check, not a replacement for the Keil build.
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
$projectDir = Join-Path $projectRoot '1_user_project'
$outputDir = Join-Path $projectRoot 'tests/build/arm'
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

if (-not $Compiler) {
    $found = Get-Command arm-none-eabi-gcc -ErrorAction SilentlyContinue
    if ($found) { $Compiler = $found.Source }
    else {
        $Compiler = 'C:/APP/STM32CubeIDE/STM32CubeIDE_1.10.1/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.10.3-2021.10.win32_1.0.0.202111181127/tools/bin/arm-none-eabi-gcc.exe'
    }
}
if (-not (Test-Path -LiteralPath $Compiler)) {
    throw 'ARM GCC not found. Pass -Compiler with the full arm-none-eabi-gcc path.'
}

[xml]$project = Get-Content -Raw -Encoding UTF8 (Join-Path $projectDir 'nrf52832_ble.uvprojx')
$target = $project.Project.Targets.Target | Where-Object TargetName -eq 'my_borad'
$controls = $target.TargetOption.TargetArmAds.Cads.VariousControls
$commonArgs = @('-mcpu=cortex-m4', '-mthumb', '-mfloat-abi=hard', '-mfpu=fpv4-sp-d16',
    '-std=c99', '-Os', '-g3', '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing',
    '-Wall', '-Wextra')
foreach ($define in $controls.Define.Split(',')) { $commonArgs += '-D' + $define }
foreach ($include in $controls.IncludePath.Split(';')) {
    $relativeInclude = [IO.Path]::GetFullPath((Join-Path $projectDir $include)).Substring($projectRoot.Length + 1)
    if ($relativeInclude -eq '8_hardware' -or $relativeInclude -eq '1_user_project') {
        $commonArgs += '-I' + $relativeInclude
    } else {
        $commonArgs += @('-isystem', $relativeInclude)
    }
}
$commonArgs += @('-isystem', '2_components/toolchain/cmsis/include')
$sources = @($target.Groups.Group.Files.File | Where-Object FileType -eq '1' | ForEach-Object {
    [IO.Path]::GetFullPath((Join-Path $projectDir $_.FilePath))
})
# Use the SDK's matching compiler implementation for its assembly error handler.
$sources = @($sources | ForEach-Object { $_.Replace('app_error_handler_keil.c', 'app_error_handler_gcc.c') })
$sources += Join-Path $projectDir 'RTE/Device/nRF52832_xxAA/system_nrf52.c'
$sources += Join-Path $projectRoot '5_modules/nrfx/mdk/gcc_startup_nrf52.S'
$failed = @()
$objects = @()
$log = @(& $Compiler --version | Select-Object -First 1)
$index = 0
foreach ($source in $sources) {
    $object = Join-Path 'tests/build/arm' ('{0:D3}_{1}.o' -f $index++, [IO.Path]::GetFileNameWithoutExtension($source))
    $objects += $object
    $fileArgs = @($commonArgs)
    if ($source -like '*8_hardware*' -or $source -eq (Join-Path $projectDir 'main.c')) {
        $fileArgs += '-Werror'
    }
    $oldPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    $relativeSource = $source.Substring($projectRoot.Length + 1)
    $diagnostics = @(& $Compiler @fileArgs '-c' $relativeSource '-o' $object 2>&1)
    $result = $LASTEXITCODE
    $ErrorActionPreference = $oldPreference
    $log += @("SOURCE: $source") + @($diagnostics | ForEach-Object { "$_" })
    if ($result -ne 0) { $failed += $source; $diagnostics | Where-Object { "$_" -match 'error:' } | Select-Object -First 8 | Write-Output }
}
$log | Set-Content -Encoding UTF8 (Join-Path $outputDir 'compile.log')
if ($failed.Count) { throw "$($failed.Count) source file(s) failed. See tests/build/arm/compile.log" }
Write-Output "ARM compile PASS: $($sources.Count) translation units; application warnings treated as errors."
$linkArgs = @('-mcpu=cortex-m4', '-mthumb', '-mfloat-abi=hard', '-mfpu=fpv4-sp-d16',
    '--specs=nano.specs', '--specs=nosys.specs', '-L5_modules/nrfx/mdk',
    '-Tscripts/check_link.ld', '-Wl,--gc-sections,-Map=tests/build/arm/check.map')
& $Compiler @linkArgs @objects '-o' 'tests/build/arm/check.elf'
if ($LASTEXITCODE -ne 0) { throw 'ARM regression link failed' }
$sizeTool = Join-Path (Split-Path -Parent $Compiler) 'arm-none-eabi-size.exe'
if (Test-Path -LiteralPath $sizeTool) { & $sizeTool 'tests/build/arm/check.elf' }
Write-Output 'ARM link PASS: all symbols resolved; FLASH/RAM linker bounds checked.'
Write-Output 'GNU regression build only. Production Keil rebuild and on-device checks are still required.'
