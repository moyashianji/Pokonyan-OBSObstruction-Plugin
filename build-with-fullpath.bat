@echo off
echo ===================================
echo Building OBS Plugin
echo ===================================

cd /d "%~dp0"

set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"

if not exist "%CMAKE_EXE%" (
    echo ERROR: CMake not found at: %CMAKE_EXE%
    echo.
    echo Please check your CMake installation.
    echo Common locations:
    echo   - C:\Program Files\CMake\bin\cmake.exe
    echo   - C:\Program Files (x86)\CMake\bin\cmake.exe
    pause
    exit /b 1
)

REM Clean build directory
if exist build (
    echo Cleaning build directory...
    rmdir /s /q build
)
mkdir build
cd build

echo.
echo === Step 1: Configuring with CMake ===
echo Using: %CMAKE_EXE%
"%CMAKE_EXE%" -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_PREFIX_PATH="D:/obs-studio/build64;C:/Qt/6.10.1/msvc2022_64" ^
    -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" ^
    ..

if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo === Step 2: Building Release ===
"%CMAKE_EXE%" --build . --config Release

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ===================================
echo BUILD SUCCESSFUL!
echo ===================================
echo.
echo Output DLL: %CD%\Release\obs-youtube-superchat-plugin.dll
echo.

cd ..
pause
