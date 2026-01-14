# OBS Plugin Build Script - Correct Paths
Write-Host "===================================" -ForegroundColor Green
Write-Host "Building OBS YouTube SuperChat Plugin" -ForegroundColor Green
Write-Host "===================================" -ForegroundColor Green
Write-Host ""

$ErrorActionPreference = "Stop"

# Set CORRECT paths
$CMAKE_EXE = "C:\Program Files\CMake\bin\cmake.exe"
$OBS_PATH = "C:/obs-studio"
$QT_PATH = "C:/Qt/6.10.1/msvc2022_64"
$VCPKG_TOOLCHAIN = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"

Write-Host "Using OBS from: $OBS_PATH" -ForegroundColor Cyan
Write-Host ""

# Clean build directory
if (Test-Path "build") {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Path "build" -Recurse -Force
}
New-Item -ItemType Directory -Path "build" | Out-Null
Set-Location "build"

try {
    # Configure
    Write-Host ""
    Write-Host "=== Step 1: Configuring with CMake ===" -ForegroundColor Cyan
    & $CMAKE_EXE -G "Visual Studio 17 2022" -A x64 `
        -DCMAKE_PREFIX_PATH="$OBS_PATH;$QT_PATH" `
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" `
        ..

    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build
    Write-Host ""
    Write-Host "=== Step 2: Building Release ===" -ForegroundColor Cyan
    & $CMAKE_EXE --build . --config Release --verbose

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    Write-Host ""
    Write-Host "===================================" -ForegroundColor Green
    Write-Host "BUILD SUCCESSFUL!" -ForegroundColor Green
    Write-Host "===================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Output DLL: $(Get-Location)\Release\obs-youtube-superchat-plugin.dll" -ForegroundColor Yellow
    Write-Host ""
}
catch {
    Write-Host ""
    Write-Host "ERROR: $_" -ForegroundColor Red
    Set-Location ..
    Read-Host "Press Enter to exit"
    exit 1
}

Set-Location ..
Read-Host "Press Enter to exit"
