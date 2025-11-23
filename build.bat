@echo off
REM OBS YouTube SuperChat Plugin Build Script for Windows

echo === OBS YouTube SuperChat Plugin Build Script ===
echo.

REM Check for vcpkg
set VCPKG_ROOT=C:\vcpkg
if exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
    echo Found vcpkg at %VCPKG_ROOT%
    set USE_VCPKG=1
) else (
    echo vcpkg not found at %VCPKG_ROOT%
    set USE_VCPKG=0
)

REM Check if OBS_BUILD_DIR is set
if "%OBS_BUILD_DIR%"=="" (
    echo Warning: OBS_BUILD_DIR is not set
    echo.
    echo Please enter your OBS Studio directory:
    echo Example: C:\obs-studio
    echo.
    set /p OBS_BUILD_DIR="OBS Studio directory: "

    if "%OBS_BUILD_DIR%"=="" (
        echo Error: Invalid OBS directory
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

REM Configure with or without vcpkg
echo Configuring...
if "%USE_VCPKG%"=="1" (
    echo Using vcpkg toolchain
    cmake -G "Visual Studio 17 2022" -A x64 ^
          -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
          -DCMAKE_PREFIX_PATH=%OBS_BUILD_DIR% ^
          ..
) else (
    echo Not using vcpkg
    cmake -G "Visual Studio 17 2022" -A x64 ^
          -DCMAKE_PREFIX_PATH=%OBS_BUILD_DIR% ^
          ..
)

if %errorlevel% neq 0 (
    echo.
    echo Configuration failed!
    echo.
    echo Common issues:
    echo - OBS Studio directory is incorrect
    echo - Qt6 is not installed
    echo - Missing dependencies (curl, nlohmann-json)
    echo.
    echo Try installing dependencies with vcpkg:
    echo   cd C:\vcpkg
    echo   .\vcpkg install qt6-base:x64-windows qt6-widgets:x64-windows curl:x64-windows
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
echo Installation options:
echo   1 - Install to user directory (recommended, no admin rights needed)
echo   2 - Install to system directory (requires administrator)
echo   3 - Skip installation
echo.
set /p INSTALL_CHOICE="Select option (1-3): "

if "%INSTALL_CHOICE%"=="1" (
    echo Installing to user directory...
    set USER_PLUGINS=%APPDATA%\obs-studio\obs-plugins\64bit
    set USER_DATA=%APPDATA%\obs-studio\obs-plugins\obs-youtube-superchat-plugin

    if not exist "%USER_PLUGINS%" mkdir "%USER_PLUGINS%"
    if not exist "%USER_DATA%" mkdir "%USER_DATA%"

    copy /Y Release\obs-youtube-superchat-plugin.dll "%USER_PLUGINS%\"
    if exist ..\data xcopy /E /I /Y ..\data "%USER_DATA%\data\"

    echo.
    echo Installation complete!
    echo Plugin installed to: %USER_PLUGINS%
) else if "%INSTALL_CHOICE%"=="2" (
    echo Installing to system directory (requires administrator)...
    cmake --install . --config Release
    if %errorlevel% neq 0 (
        echo.
        echo Installation failed! Try running as administrator.
    ) else (
        echo Installation complete!
    )
) else (
    echo Skipping installation
    echo.
    echo To install manually to user directory:
    echo   copy Release\obs-youtube-superchat-plugin.dll %%APPDATA%%\obs-studio\obs-plugins\64bit\
    echo   xcopy /E /I data %%APPDATA%%\obs-studio\obs-plugins\obs-youtube-superchat-plugin\data\
)

echo.
echo ========================================
echo Build complete!
echo ========================================
if not "%INSTALL_CHOICE%"=="3" (
    echo Please restart OBS Studio to load the plugin.
    echo.
    echo After restart, check:
    echo   Tools -^> YouTube SuperChat Plugin Settings
)
echo.

cd ..
pause
