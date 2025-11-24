# OBS Plugin Build Script - Final Version
Write-Host "===================================" -ForegroundColor Green
Write-Host "Building OBS YouTube SuperChat Plugin" -ForegroundColor Green
Write-Host "===================================" -ForegroundColor Green
Write-Host ""

$ErrorActionPreference = "Stop"

# Set paths - Use OBS BUILD directory directly
$CMAKE_EXE = "C:\Program Files\CMake\bin\cmake.exe"
$OBS_BUILD = "C:/obs-studio/build"
$QT_PATH = "C:/obs-studio/.deps/obs-deps-qt6-2025-08-23-x64"
$VCPKG_TOOLCHAIN = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"

Write-Host "Using OBS BUILD from: $OBS_BUILD" -ForegroundColor Cyan
Write-Host ""

# Clean build directory
if (Test-Path "build") {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Path "build" -Recurse -Force
}
New-Item -ItemType Directory -Path "build" | Out-Null
Set-Location "build"

try {
    # Configure - Use OBS build directory directly
    Write-Host ""
    Write-Host "=== Step 1: Configuring with CMake ===" -ForegroundColor Cyan
    & $CMAKE_EXE -G "Visual Studio 17 2022" -A x64 `
        -DCMAKE_PREFIX_PATH="$OBS_BUILD;$QT_PATH" `
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" `
        ..

    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build
    Write-Host ""
    Write-Host "=== Step 2: Building Release ===" -ForegroundColor Cyan
    & $CMAKE_EXE --build . --config Release

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    Write-Host ""
    Write-Host "===================================" -ForegroundColor Green
    Write-Host "BUILD SUCCESSFUL!" -ForegroundColor Green
    Write-Host "===================================" -ForegroundColor Green
    Write-Host ""
    $dllPath = "$(Get-Location)\Release\obs-youtube-superchat-plugin.dll"
    if (Test-Path $dllPath) {
        Write-Host "Output DLL: $dllPath" -ForegroundColor Yellow
    } else {
        Write-Host "Warning: DLL not found at expected location" -ForegroundColor Yellow
        Write-Host "Searching for DLL..." -ForegroundColor Yellow
        $foundDlls = Get-ChildItem -Path . -Filter "*.dll" -Recurse
        foreach ($dll in $foundDlls) {
            Write-Host "  Found: $($dll.FullName)" -ForegroundColor Cyan
        }
    }
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
