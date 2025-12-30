# OBS Plugin Build Script
$ErrorActionPreference = "Continue"

Write-Host "===================================" -ForegroundColor Green
Write-Host "Building OBS YouTube SuperChat Plugin" -ForegroundColor Green
Write-Host "===================================" -ForegroundColor Green

# Visual Studio 2022 Build Tools paths
$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
$msvcPath = "$vsPath\VC\Tools\MSVC\14.44.35207"
$sdkPath = "C:\Program Files (x86)\Windows Kits\10"
$sdkVersion = "10.0.22621.0"

Write-Host "SDK Version: $sdkVersion" -ForegroundColor Cyan
Write-Host "MSVC Path: $msvcPath" -ForegroundColor Cyan

# Set environment - include Windows SDK bin directory for rc.exe
$env:PATH = "$sdkPath\bin\$sdkVersion\x64;$msvcPath\bin\Hostx64\x64;C:\Users\moyashi\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Program Files\CMake\bin;$env:PATH"
$env:INCLUDE = "$msvcPath\include;$sdkPath\Include\$sdkVersion\ucrt;$sdkPath\Include\$sdkVersion\shared;$sdkPath\Include\$sdkVersion\um;$sdkPath\Include\$sdkVersion\winrt"
$env:LIB = "$msvcPath\lib\x64;$sdkPath\Lib\$sdkVersion\ucrt\x64;$sdkPath\Lib\$sdkVersion\um\x64"

Write-Host ""
Write-Host "Checking tools..." -ForegroundColor Cyan
& ninja --version
& cmake --version | Select-Object -First 1

# Check compiler - use forward slashes for CMake
$clPath = "$msvcPath\bin\Hostx64\x64\cl.exe"
$clPathCMake = $clPath -replace '\\', '/'
Write-Host "Testing cl.exe..." -ForegroundColor Cyan
if (Test-Path $clPath) {
    Write-Host "Found: $clPath" -ForegroundColor Green
} else {
    Write-Host "cl.exe not found at $clPath" -ForegroundColor Red
    exit 1
}

# Check rc.exe - use forward slashes for CMake
$rcPath = "$sdkPath\bin\$sdkVersion\x64\rc.exe"
$rcPathCMake = $rcPath -replace '\\', '/'
Write-Host "Testing rc.exe..." -ForegroundColor Cyan
if (Test-Path $rcPath) {
    Write-Host "Found: $rcPath" -ForegroundColor Green
} else {
    Write-Host "rc.exe not found at $rcPath" -ForegroundColor Red
    exit 1
}

# Set paths - use forward slashes for CMake
$OBS_SRC = "C:/obs-studio-src"
$QT_PATH = "C:/Qt/6.6.3/msvc2019_64"

Write-Host ""
Write-Host "Using:" -ForegroundColor Cyan
Write-Host "  OBS Source: $OBS_SRC"
Write-Host "  Qt Path: $QT_PATH"

# Navigate to project
Set-Location "C:\Users\moyashi\Pokonyan-OBSObstruction-Plugin"

# Clean and create build directory
if (Test-Path "build") {
    Remove-Item -Path "build" -Recurse -Force
}
New-Item -ItemType Directory -Path "build" | Out-Null
Set-Location "build"

# Configure with CMake - use static runtime
Write-Host ""
Write-Host "=== Step 1: Configuring with CMake ===" -ForegroundColor Cyan

& cmake -G "Ninja" `
    -DCMAKE_BUILD_TYPE=Release `
    "-DCMAKE_C_COMPILER=$clPathCMake" `
    "-DCMAKE_CXX_COMPILER=$clPathCMake" `
    "-DCMAKE_RC_COMPILER=$rcPathCMake" `
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded" `
    "-DCMAKE_PREFIX_PATH=$OBS_SRC;$QT_PATH" `
    ..

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

# Build
Write-Host ""
Write-Host "=== Step 2: Building ===" -ForegroundColor Cyan
& cmake --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "Build failed!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Write-Host ""
Write-Host "===================================" -ForegroundColor Green
Write-Host "BUILD SUCCESSFUL!" -ForegroundColor Green
Write-Host "===================================" -ForegroundColor Green
Write-Host ""
Write-Host "Looking for output DLL..." -ForegroundColor Yellow
Get-ChildItem -Path . -Filter "*.dll" -Recurse

Set-Location ..
