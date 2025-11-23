@echo off
REM OBS YouTube SuperChat Plugin Build Script for Windows

echo === OBS YouTube SuperChat Plugin Build Script ===
echo.

REM Check if OBS_BUILD_DIR is set
if "%OBS_BUILD_DIR%"=="" (
    echo Warning: OBS_BUILD_DIR is not set
    echo Please set it to your OBS Studio build directory:
    echo   set OBS_BUILD_DIR=C:\path\to\obs-studio\build
    echo.
    set /p OBS_BUILD_DIR="Enter OBS Studio build directory: "

    if "%OBS_BUILD_DIR%"=="" (
        echo Error: Invalid OBS build directory
        exit /b 1
    )
)

REM Create build directory
set BUILD_DIR=build
if exist %BUILD_DIR% (
    echo Cleaning existing build directory...
    rmdir /s /q %BUILD_DIR%
)

mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Configure
echo Configuring...
cmake -G "Visual Studio 17 2022" ^
      -DCMAKE_PREFIX_PATH=%OBS_BUILD_DIR% ^
      -DCMAKE_BUILD_TYPE=Release ^
      ..

if %errorlevel% neq 0 (
    echo Configuration failed!
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

REM Install
echo.
set /p INSTALL="Do you want to install the plugin? (y/n) "
if /i "%INSTALL%"=="y" (
    echo Installing...
    cmake --install . --config Release
    echo Installation complete!
) else (
    echo Skipping installation
    echo To install manually, run:
    echo   cd %BUILD_DIR% ^&^& cmake --install . --config Release
)

echo.
echo Build complete!
echo Please restart OBS Studio to load the plugin.

cd ..
pause
