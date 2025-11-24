@echo off
echo ===================================
echo CMake Installer Launcher
echo ===================================
echo.
echo Opening CMake download page...
echo.
echo Please:
echo 1. Download "cmake-3.XX.X-windows-x86_64.msi"
echo 2. Run the installer
echo 3. Select "Add CMake to system PATH for all users"
echo 4. Complete installation
echo 5. Restart this terminal/Visual Studio
echo.

start "" "https://cmake.org/download/"

echo.
echo After installation, close Visual Studio and run open-in-vs.bat again!
echo.
pause
