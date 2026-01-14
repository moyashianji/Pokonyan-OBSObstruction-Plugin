@echo off
setlocal enabledelayedexpansion

REM Simple build script without interactive prompts
echo === Building OBS YouTube SuperChat Plugin ===

REM Set paths
set "OBS_PATH=D:\obs-studio\build64"
set "QT_PATH=C:\Qt\6.10.1\msvc2022_64"
set "VCPKG_PATH=C:\vcpkg\scripts\buildsystems\vcpkg.cmake"

echo OBS Path: %OBS_PATH%
echo Qt Path: %QT_PATH%
echo VCPKG Toolchain: %VCPKG_PATH%
echo.

REM Clean build directory
if exist build (
    echo Cleaning build directory...
    rd /s /q build
)
mkdir build
cd build

echo Configuring CMake...
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%OBS_PATH%;%QT_PATH%" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_PATH%" ..

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building...
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ===== Build Successful! =====
echo.
echo Output DLL: %CD%\Release\obs-youtube-superchat-plugin.dll
echo.

cd ..
pause
