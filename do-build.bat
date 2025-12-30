@echo off
setlocal EnableDelayedExpansion

echo ===================================
echo Building OBS YouTube SuperChat Plugin
echo ===================================

:: Set up Visual Studio environment (VS Insiders)
echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"

:: Add tools to PATH
set PATH=C:\Users\moyashi\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Program Files\CMake\bin;%PATH%

echo.
echo Checking tools...
where ninja
where cmake
where cl

:: Set paths
set OBS_SRC=C:\obs-studio-src
set QT_PATH=C:\Qt\6.6.3\msvc2019_64
set CURL_PATH=C:\deps\curl-8.12.1_1-win64-mingw

echo.
echo Using:
echo   OBS Source: %OBS_SRC%
echo   Qt Path: %QT_PATH%
echo   cURL Path: %CURL_PATH%
echo.

:: Create and enter build directory
cd /d "C:\Users\moyashi\Pokonyan-OBSObstruction-Plugin"
if exist build rmdir /s /q build
mkdir build
cd build

:: Configure with CMake
echo.
echo === Step 1: Configuring with CMake ===
cmake -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%OBS_SRC%;%QT_PATH%" ^
    -DCURL_INCLUDE_DIR="%CURL_PATH%\include" ^
    ..

if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    cd ..
    exit /b 1
)

:: Build
echo.
echo === Step 2: Building ===
cmake --build . --config Release

if errorlevel 1 (
    echo.
    echo Build failed!
    cd ..
    exit /b 1
)

echo.
echo ===================================
echo BUILD SUCCESSFUL!
echo ===================================
echo.
echo Looking for output DLL...
dir /s *.dll 2>nul

cd ..
exit /b 0
