<#
    Builds DB-WEAVE on Windows, assembles the redistributable tree
    via windeployqt, and produces dbweave_setup.exe with NSIS.

    Prerequisites:
        * Qt 6.5+ MSVC or MinGW installation on PATH (qmake, cmake,
          windeployqt all reachable from the VS Dev Prompt or a
          qt-env shell).
        * CMake 3.21+ and Ninja (or MSBuild).
        * NSIS (makensis) on PATH.
        * Optionally: Qt6SerialPort dev kit if you pass -WithLoom.

    Usage:
        # From a Qt x64 Developer Command Prompt or with Qt on PATH:
        powershell -File packaging\windows\build_installer.ps1
        # or
        powershell -File packaging\windows\build_installer.ps1 -WithLoom
#>
param(
    [switch]$WithLoom
)

$ErrorActionPreference = 'Stop'

$Root     = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$BuildDir = Join-Path $Root 'build-windows'
$StageDir = Join-Path $Root 'dist\windeployqt'
$DistDir  = Join-Path $Root 'dist'

$loom = if ($WithLoom) { 'ON' } else { 'OFF' }

Write-Host "==> configure (loom=$loom)"
cmake -S $Root -B $BuildDir -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DDBWEAVE_BUILD_TESTS=OFF `
    "-DDBWEAVE_BUILD_LOOM=$loom"

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
Push-Location (Join-Path $Root 'packaging\windows')
try {
    & makensis 'dbweave.nsi'
    if ($LASTEXITCODE -ne 0) { throw 'makensis failed' }
    Move-Item -Force 'dbweave_setup.exe' $DistDir
} finally {
    Pop-Location
}

Write-Host "==> done. Installer in $DistDir\dbweave_setup.exe"
