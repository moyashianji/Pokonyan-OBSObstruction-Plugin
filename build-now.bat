@echo off
echo ===================================
echo Building OBS Plugin with CMake
echo ===================================

cd /d "%~dp0"

REM Clean build directory
if exist build (
    echo Cleaning build directory...
    rmdir /s /q build
)
mkdir build
cd build

echo.
echo === Step 1: Configuring with CMake ===
cmake -G "Visual Studio 17 2022" -A x64 ^
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
cmake --build . --config Release

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
