@echo off
setlocal EnableDelayedExpansion

echo ===================================
echo Building OBS YouTube SuperChat Plugin
echo ===================================

:: Set up Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"

:: Set paths
set CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe
set OBS_SRC=C:/obs-studio-src
set OBS_INSTALL=C:/Program Files/obs-studio

:: Create and enter build directory
if exist build rmdir /s /q build
mkdir build
cd build

:: Configure with CMake
echo.
echo === Configuring with CMake ===
"%CMAKE_EXE%" -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%OBS_INSTALL%" ^
    -Dlibobs_DIR="%OBS_SRC%/libobs" ^
    ..

if errorlevel 1 (
    echo CMake configuration failed
    cd ..
    pause
    exit /b 1
)

:: Build
echo.
echo === Building ===
"%CMAKE_EXE%" --build . --config Release

if errorlevel 1 (
    echo Build failed
    cd ..
    pause
    exit /b 1
)

echo.
echo ===================================
echo BUILD SUCCESSFUL!
echo ===================================
dir /s *.dll

cd ..
pause
