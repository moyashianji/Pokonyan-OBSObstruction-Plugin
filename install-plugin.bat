@echo off
echo ===================================
echo OBS YouTube SuperChat Plugin Installer
echo ===================================
echo.

set "PLUGIN_DLL=%~dp0build\Release\obs-youtube-superchat-plugin.dll"
set "OBS_PLUGINS_DIR=C:\Program Files\obs-studio\obs-plugins\64bit"

if not exist "%PLUGIN_DLL%" (
    echo ERROR: Plugin DLL not found at: %PLUGIN_DLL%
    echo Please build the plugin first using build-final.ps1
    pause
    exit /b 1
)

echo Plugin DLL: %PLUGIN_DLL%
echo Target Directory: %OBS_PLUGINS_DIR%
echo.

if not exist "%OBS_PLUGINS_DIR%" (
    echo ERROR: OBS plugins directory not found: %OBS_PLUGINS_DIR%
    echo.
    echo If you're using portable OBS, please manually copy the DLL to:
    echo   your-obs-folder\obs-plugins\64bit\obs-youtube-superchat-plugin.dll
    echo.
    pause
    exit /b 1
)

echo Copying plugin to OBS...
copy /Y "%PLUGIN_DLL%" "%OBS_PLUGINS_DIR%\obs-youtube-superchat-plugin.dll"

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Failed to copy plugin. You may need to run this script as Administrator.
    echo Right-click this batch file and select "Run as administrator"
    pause
    exit /b 1
)

echo.
echo ===================================
echo INSTALLATION SUCCESSFUL!
echo ===================================
echo.
echo Plugin installed to: %OBS_PLUGINS_DIR%\obs-youtube-superchat-plugin.dll
echo.
echo Please restart OBS Studio to load the plugin.
echo.
pause
