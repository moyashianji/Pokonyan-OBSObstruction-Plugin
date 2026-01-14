@echo off
chcp 65001 >nul
echo ====================================
echo Qt 6.6.3 TLS プラグインインストーラー
echo ====================================
echo.
echo このスクリプトは以下を実行します:
echo 1. Qt MaintenanceToolを使ってQt 6.6.3をインストール
echo 2. Qt 6.6.3のTLSプラグインをOBSにコピー
echo.
echo OBS 30.2.3はQt 6.6.3を使用しているため、
echo 同じバージョンのTLSプラグインが必要です。
echo.
pause

REM Check for admin rights
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo.
    echo エラー: 管理者権限が必要です
    echo.
    echo このバッチファイルを右クリックして
    echo 「管理者として実行」を選択してください。
    echo.
    pause
    exit /b 1
)

REM Run PowerShell script
powershell.exe -ExecutionPolicy Bypass -File "%~dp0install-qt663-auto.ps1"

echo.
echo スクリプト完了
pause
