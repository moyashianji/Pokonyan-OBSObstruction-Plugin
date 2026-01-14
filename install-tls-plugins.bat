@echo off
echo ===================================
echo Install Qt TLS Plugins for OBS
echo ===================================
echo.

set "SOURCE=C:\obs-studio\.deps\obs-deps-qt6-2025-08-23-x64\plugins\tls"
set "DEST=C:\Program Files\obs-studio\bin\64bit\tls"

if not exist "%SOURCE%" (
    echo ERROR: Source TLS plugins not found at: %SOURCE%
    pause
    exit /b 1
)

echo Source: %SOURCE%
echo Destination: %DEST%
echo.

echo Creating tls directory in OBS...
mkdir "%DEST%" 2>nul

echo Copying TLS plugins...
copy /Y "%SOURCE%\*.dll" "%DEST%\"

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Failed to copy TLS plugins.
    echo Please run this script as Administrator.
    echo Right-click this file and select "Run as administrator"
    pause
    exit /b 1
)

echo.
echo ===================================
echo TLS PLUGINS INSTALLED!
===================================
echo.
echo Restart OBS to apply changes.
echo.
pause
