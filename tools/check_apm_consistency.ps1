$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$templateRoot = Join-Path $projectRoot 'SampleCode\Template'
$sampleNames = @('Colsonic_16k_I2S_I2C', 'Nuvoton_demo_AEC_16k_0.0.2a')
$pairedFiles = @(
    'colsonic_apm_16k_320.lib', 'colsonicdata.bin',
    'colsonic_apm.h', 'colsonic_def.h', 'colsonic_def.c',
    'alg_type.h', 'ColsonicData_Keil.s'
)
$expectedLibrary = '3b8657ddd14d028637f8ccfc25ae9d5d9b7641bc332a7c38cd642fa2ac52fc18'
$expectedProfile = 'd72230e86045e94dc6c6e1e278f07f8059c3dd702e8f560af71c8109d81d8bf9'

foreach ($file in $pairedFiles) {
    $firstPath = Join-Path $templateRoot ($sampleNames[0] + '\Colsonic\' + $file)
    $secondPath = Join-Path $templateRoot ($sampleNames[1] + '\Colsonic\' + $file)
    $firstHash = (Get-FileHash -LiteralPath $firstPath -Algorithm SHA256).Hash.ToLowerInvariant()
    $secondHash = (Get-FileHash -LiteralPath $secondPath -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($firstHash -ne $secondHash) { throw "APM pair differs: $file" }
    if ($file -eq 'colsonic_apm_16k_320.lib' -and $firstHash -ne $expectedLibrary) {
        throw 'The library pair differs from the selected USB baseline.'
    }
    if ($file -eq 'colsonicdata.bin' -and $firstHash -ne $expectedProfile) {
        throw 'The parameter profile differs from the selected USB baseline.'
    }
    Write-Output "PASS $file $firstHash"
}

foreach ($sample in $sampleNames) {
    $sampleRoot = Join-Path $templateRoot $sample
    $keilRoot = Join-Path $sampleRoot 'KEIL'
    $projectFile = @(Get-ChildItem -LiteralPath $keilRoot -Filter '*.uvprojx')
    if ($projectFile.Count -ne 1) { throw "Expected one Keil project: $sample" }
    [xml]$project = Get-Content -LiteralPath $projectFile[0].FullName -Raw
    $libraries = @($project.SelectNodes('//File[FileName="colsonic_apm_16k_320.lib"]'))
    if ($libraries.Count -ne 1) { throw "Expected one APM library reference: $sample" }
    $linkedLibrary = [IO.Path]::GetFullPath((Join-Path $keilRoot $libraries[0].FilePath))
    $expectedPath = [IO.Path]::GetFullPath((Join-Path $sampleRoot 'Colsonic\colsonic_apm_16k_320.lib'))
    if ($linkedLibrary -ne $expectedPath) { throw "Unexpected linked library: $sample" }
    $enumSetting = $project.SelectSingleNode('//TargetArmAds/Cads/EnumInt')
    if ($null -eq $enumSetting -or $enumSetting.InnerText -ne '0') {
        throw "The selected module-table ABI requires short enums: $sample"
    }
    $definitions = Get-Content -LiteralPath (Join-Path $sampleRoot 'Colsonic\colsonic_def.h') -Raw
    if ($definitions -notmatch '#define\s+COLSONIC_BUF_LEN_MODE\s+\(148000\)' -or
        $definitions -notmatch '#define\s+COLSONIC_AEC_LEN\s+\(800\)') {
        throw "APM memory/AEC configuration differs from the selected baseline: $sample"
    }
    $device = $project.SelectSingleNode('//TargetCommonOption/Device')
    if ($null -eq $device -or $device.InnerText -ne 'I94124') {
        throw "Expected I94124 target device: $sample"
    }
    $cpu = $project.SelectSingleNode('//TargetCommonOption/Cpu')
    if ($null -eq $cpu -or $cpu.InnerText -notmatch 'IRAM\(0x20000000-0x2002FFFF\)') {
        throw "CPU memory description differs from I94124 SRAM: $sample"
    }
    $areas = @(
        @{ Name = 'IRAM'; Start = 0x20000000; Size = 0x30000 },
        @{ Name = 'OCR_RVCT9'; Start = 0x20000000; Size = 0x30000 },
        @{ Name = 'IROM'; Start = 0x00000000; Size = 0x80000 },
        @{ Name = 'OCR_RVCT4'; Start = 0x00000000; Size = 0x80000 }
    )
    foreach ($area in $areas) {
        $node = $project.SelectSingleNode('//OnChipMemories/' + $area.Name)
        if ($null -eq $node -or
            [Convert]::ToUInt32([string]$node.StartAddress, 16) -ne $area.Start -or
            [Convert]::ToUInt32([string]$node.Size, 16) -ne $area.Size) {
            throw "Unexpected I94124 memory area $($area.Name): $sample"
        }
    }
    $layout = $project.SelectSingleNode('//TargetArmAds/LDads/umfTarg')
    if ($null -eq $layout -or $layout.InnerText -ne '1') {
        throw "Linker must use the target memory layout: $sample"
    }
    foreach ($selection in @('Ir1Chk', 'Im1Chk')) {
        $enabled = $project.SelectSingleNode('//TargetArmAds/ArmAdsMisc/' + $selection)
        if ($null -eq $enabled -or $enabled.InnerText -ne '1') {
            throw "Primary target memory area is disabled ($selection): $sample"
        }
    }
    $startup = Get-Content -LiteralPath (Join-Path $keilRoot 'startup_I94100.s') -Raw
    if ($startup -notmatch '(?m)^Stack_Size\s+EQU\s+0x00001000\s*$') {
        throw "Expected the shared 4 KiB startup stack reservation: $sample"
    }
    Write-Output "PASS linked library $sample"
    Write-Output "PASS I94124 target: 192 KiB SRAM, 512 KiB APROM, target memory layout enabled ($sample)"
}
Write-Output 'PASS: both examples use matching APM files and the I94124 target memory layout.'
