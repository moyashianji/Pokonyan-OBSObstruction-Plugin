@echo off
chcp 65001 >nul
echo ===================================
echo YouTube SuperChat Plugin インストーラー
echo ===================================
echo.
echo このプラグインは、YouTubeのスーパーチャットと
echo チャットメッセージに反応して画面に妨害エフェクトを
echo 発生させるOBS Studio用プラグインです。
echo.
echo ===================================
echo.

REM Check if running as administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo エラー: 管理者権限が必要です
    echo.
    echo このバッチファイルを右クリックして
    echo 「管理者として実行」を選択してください。
    echo.
    pause
    exit /b 1
)

set "PLUGIN_DLL=%~dp0obs-youtube-superchat-plugin.dll"
set "TLS_SOURCE=%~dp0tls"
set "OBS_PLUGINS_DIR=C:\Program Files\obs-studio\obs-plugins\64bit"
set "OBS_BIN_DIR=C:\Program Files\obs-studio\bin\64bit"

echo [1/3] プラグインファイルの確認...
if not exist "%PLUGIN_DLL%" (
    echo エラー: プラグインDLLが見つかりません
    echo 場所: %PLUGIN_DLL%
    pause
    exit /b 1
)
echo ✓ プラグインDLL確認完了

echo.
echo [2/3] OBSディレクトリの確認...
if not exist "%OBS_PLUGINS_DIR%" (
    echo エラー: OBS Studioが見つかりません
    echo.
    echo OBS Studioがインストールされていることを確認してください。
    echo 標準パス: C:\Program Files\obs-studio
    echo.
    pause
    exit /b 1
)
echo ✓ OBSディレクトリ確認完了

echo.
echo [3/3] インストール実行中...
echo.

REM Install plugin DLL
echo - プラグインDLLをコピー中...
copy /Y "%PLUGIN_DLL%" "%OBS_PLUGINS_DIR%\obs-youtube-superchat-plugin.dll" >nul
if %errorlevel% neq 0 (
    echo エラー: プラグインDLLのコピーに失敗しました
    pause
    exit /b 1
)
echo   ✓ プラグインDLLインストール完了

REM Install TLS plugins if available
if exist "%TLS_SOURCE%" (
    echo - TLSプラグインをコピー中...
    if not exist "%OBS_BIN_DIR%\tls" mkdir "%OBS_BIN_DIR%\tls"
    copy /Y "%TLS_SOURCE%\*.dll" "%OBS_BIN_DIR%\tls\" >nul
    if %errorlevel% equ 0 (
        echo   ✓ TLSプラグインインストール完了
    ) else (
        echo   ⚠ TLSプラグインのコピーに失敗（続行可能）
    )
)

echo.
echo ===================================
echo インストール完了！
echo ===================================
echo.
echo 次の手順:
echo 1. OBS Studioを再起動
echo 2. ツール → YouTube SuperChat Plugin Settings を開く
echo 3. YouTube API Keyを取得して設定
echo    (取得方法は README.txt を参照)
echo 4. Video IDを入力
echo 5. 「保存」→「モニタリング開始」をクリック
echo.
echo 詳細は README.txt をご覧ください。
echo.
pause
