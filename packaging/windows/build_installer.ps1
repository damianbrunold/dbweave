<#
    Builds DB-WEAVE on Windows, assembles the redistributable tree
    via windeployqt, and produces dbweave_setup.exe with NSIS.

    Prerequisites:
        * Qt 6.5+ MSVC or MinGW installation on PATH (qmake, cmake,
          windeployqt all reachable from the VS Dev Prompt or a
          qt-env shell). Qt6SerialPort is part of the default Qt
          install.
        * CMake 3.21+ and Ninja (or MSBuild).
        * NSIS (makensis) on PATH.

    Usage:
        # From a Qt x64 Developer Command Prompt or with Qt on PATH:
        powershell -File packaging\windows\build_installer.ps1
        # or, to skip the loom/serial-port module:
        powershell -File packaging\windows\build_installer.ps1 -NoLoom
#>
param(
    [switch]$NoLoom
)

$ErrorActionPreference = 'Stop'

$Root     = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$BuildDir = Join-Path $Root 'build-windows'
$StageDir = Join-Path $Root 'dist\windeployqt'
$DistDir  = Join-Path $Root 'dist'

# Note: PowerShell variables are case-insensitive, so the local
# variable name must not collide with the $NoLoom switch parameter
# declared above -- assigning a plain 'OFF' string back into the
# switch would trigger a SwitchParameter conversion failure. Use a
# distinct name.
$NoLoomArg = if ($NoLoom.IsPresent) { 'ON' } else { 'OFF' }

Write-Host "==> configure (no-loom=$NoLoomArg)"
# Pass every CMake -D as a quoted string so PowerShell doesn't try
# to interpret the bare ON / OFF tails.
cmake -S $Root -B $BuildDir -G Ninja `
    '-DCMAKE_BUILD_TYPE=Release' `
    '-DDBWEAVE_BUILD_TESTS=OFF' `
    "-DDBWEAVE_NO_LOOM=$NoLoomArg"

Write-Host '==> build'
cmake --build $BuildDir

Write-Host '==> install + windeployqt'
if (Test-Path $StageDir) { Remove-Item $StageDir -Recurse -Force }
cmake --install $BuildDir --prefix $StageDir

$Exe = Join-Path $StageDir 'dbweave.exe'
if (-not (Test-Path $Exe)) { throw "Built executable not found at $Exe" }

# windeployqt copies Qt DLLs + plugins next to the exe.
windeployqt --release --no-translations --no-opengl-sw --no-system-d3d-compiler $Exe

# Drop docs into the install dir so the NSIS script can pick them up.
Copy-Item (Join-Path $Root 'LICENSE')           $StageDir -Force
if (Test-Path (Join-Path $Root 'dbw_manual.pdf'))    { Copy-Item (Join-Path $Root 'dbw_manual.pdf')    $StageDir -Force }
if (Test-Path (Join-Path $Root 'dbw_handbuch.pdf'))  { Copy-Item (Join-Path $Root 'dbw_handbuch.pdf')  $StageDir -Force }

Write-Host '==> makensis'
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

# Pull the version out of the top-level CMakeLists.txt so the
# installer matches the in-app version without a second source of
# truth. Mirrors the awk-based extraction in build_dmg.sh.
$CMakeLists = Join-Path $Root 'CMakeLists.txt'
$VersionMatch = Select-String -Path $CMakeLists -Pattern 'VERSION\s+(\d+\.\d+\.\d+)' | Select-Object -First 1
if (-not $VersionMatch) { throw "Could not parse VERSION from $CMakeLists" }
$AppVer = $VersionMatch.Matches[0].Groups[1].Value
Write-Host "    APPVER=$AppVer"

Push-Location (Join-Path $Root 'packaging\windows')
try {
    & makensis "/DAPPVER=$AppVer" 'dbweave.nsi'
    if ($LASTEXITCODE -ne 0) { throw 'makensis failed' }
    Move-Item -Force 'dbweave_setup.exe' $DistDir
} finally {
    Pop-Location
}

Write-Host "==> done. Installer in $DistDir\dbweave_setup.exe"
