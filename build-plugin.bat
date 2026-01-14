@echo off
setlocal EnableDelayedExpansion

echo ===================================
echo Building OBS YouTube SuperChat Plugin
echo ===================================

:: Set up Visual Studio environment (VS Insiders)
call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"

:: Set paths
set CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe
set OBS_SRC=C:\obs-studio-src
set QT_PATH=C:\Qt\6.6.3\msvc2019_64
set CURL_PATH=C:\deps\curl-8.12.1_1-win64-mingw
set JSON_PATH=C:\deps

:: Add ninja to PATH
set PATH=%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;%PATH%

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
"%CMAKE_EXE%" -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%OBS_SRC%;%QT_PATH%" ^
    -DCURL_INCLUDE_DIR="%CURL_PATH%\include" ^
    -DCURL_LIBRARY="%CURL_PATH%\lib\libcurl.a" ^
    ..

if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

:: Build
echo.
echo === Step 2: Building ===
"%CMAKE_EXE%" --build . --config Release

if errorlevel 1 (
    echo.
    echo Build failed!
    cd ..
    pause
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
pause
