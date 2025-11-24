@echo off
chcp 65001 >nul
echo ====================================
echo Qt TLS プラグインインストーラー
echo ====================================
echo.
echo OBSのQt 6.8.3に対応したTLSプラグインをインストールします
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

set "TLS_SOURCE=C:\obs-studio\.deps\obs-deps-qt6-2025-08-23-x64\plugins\tls"
set "OBS_BIN_DIR=C:\Program Files\obs-studio\bin\64bit"

echo [1/3] TLSプラグインの確認...
if not exist "%TLS_SOURCE%\qschannelbackend.dll" (
    echo エラー: TLSプラグインが見つかりません
    echo 場所: %TLS_SOURCE%
    pause
    exit /b 1
)
echo ✓ TLSプラグイン確認完了

echo.
echo [2/3] OBSディレクトリの確認...
if not exist "%OBS_BIN_DIR%" (
    echo エラー: OBS Studioが見つかりません
    echo 場所: %OBS_BIN_DIR%
    pause
    exit /b 1
)
echo ✓ OBSディレクトリ確認完了

echo.
echo [3/3] TLSプラグインをコピー中...
echo.

REM Create TLS directory
if not exist "%OBS_BIN_DIR%\tls" (
    echo - TLSディレクトリを作成中...
    mkdir "%OBS_BIN_DIR%\tls"
)

REM Remove old TLS plugins
echo - 古いTLSプラグインを削除中...
del /Q "%OBS_BIN_DIR%\tls\*.dll" 2>nul

REM Copy TLS plugins (release versions only, not debug versions)
echo - TLSプラグインをコピー中...
copy /Y "%TLS_SOURCE%\qschannelbackend.dll" "%OBS_BIN_DIR%\tls\" >nul
copy /Y "%TLS_SOURCE%\qcertonlybackend.dll" "%OBS_BIN_DIR%\tls\" >nul

if %errorlevel% neq 0 (
    echo エラー: コピーに失敗しました
    pause
    exit /b 1
)

echo   ✓ TLSプラグインインストール完了

echo.
echo ====================================
echo インストール完了！
echo ====================================
echo.
echo コピーされたファイル:
echo   - qschannelbackend.dll (Windows TLS backend)
echo   - qcertonlybackend.dll (Certificate-only backend)
echo.
echo 保存先: %OBS_BIN_DIR%\tls\
echo Qtバージョン: 6.8.3 (OBSと一致)
echo.
echo ====================================
echo 次のステップ:
echo ====================================
echo 1. OBS Studioを再起動
echo 2. ツール → YouTube SuperChat Plugin Settings
echo 3. 「モニタリング開始」をクリック
echo 4. OBSログで「TLS initialization failed」が出ないことを確認
echo.
pause
