$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $root
[xml]$project = Get-Content '1_user/coolkit_test_board_stm32f407.uvprojx'
[xml]$original = Get-Content '0_readme/review_backup/1_user/coolkit_test_board_stm32f407.uvprojx'
$target = $project.Project.Targets.Target | Select-Object -First 1
$all = @($target.Groups.Group.Files.File | ForEach-Object {
    $resolved = [IO.Path]::GetFullPath((Join-Path "$root/1_user" $_.FilePath))
    if (!(Test-Path -LiteralPath $resolved)) { throw "Missing project file: $resolved" }
    $resolved
})
$compiled = @($all | Where-Object { $_ -match '\.c$' } | Sort-Object)
$baseline = @($original.Project.Targets.Target.Groups.Group.Files.File | Where-Object {
    $_.FilePath -match '\.c$'
} | ForEach-Object {
    [IO.Path]::GetFullPath((Join-Path "$root/1_user" $_.FilePath))
} | Sort-Object)
if (Compare-Object $baseline $compiled) { throw 'Production C source set differs from the original.' }
if (($compiled | Select-Object -Unique).Count -ne $compiled.Count) { throw 'Duplicate C source in Keil project.' }
$eide = [IO.File]::ReadAllText((Join-Path $root '00_eide/.eide/eide.yml'))
$eideFiles = @([regex]::Matches($eide, '(?m)^\s+- path: ([^\r\n]+\.c)\s*$') | ForEach-Object {
    [IO.Path]::GetFullPath((Join-Path "$root/00_eide" $_.Groups[1].Value.Trim()))
} | Sort-Object)
if (Compare-Object $compiled $eideFiles) { throw 'Keil/EIDE C source lists do not agree.' }
$scatter = $target.TargetOption.TargetArmAds.LDads.ScatterFile
if ($scatter -ne '.\stm32f407_memory.sct' -or $eide -notmatch 'scatterFilePath: ../1_user/stm32f407_memory.sct') {
    throw 'Keil/EIDE must use the stable linker script.'
}
if (!(Test-Path '1_user/stm32f407_memory.sct')) { throw 'Linker script missing.' }
Write-Output ("PASS: {0} existing C files, no duplicates, Keil/EIDE agree, stable linker script." -f $compiled.Count)
