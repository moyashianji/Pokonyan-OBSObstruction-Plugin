# Qt 6.6.3 Automatic Installer for OBS TLS Support
Write-Host "====================================" -ForegroundColor Cyan
Write-Host "Qt 6.6.3 自動インストーラー" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

$maintenanceTool = "C:\Qt\MaintenanceTool.exe"
$qt663Path = "C:\Qt\6.6.3\msvc2019_64"
$obsBinDir = "C:\Program Files\obs-studio\bin\64bit"

# Check if MaintenanceTool exists
if (!(Test-Path $maintenanceTool)) {
    Write-Host "エラー: Qt MaintenanceToolが見つかりません" -ForegroundColor Red
    Write-Host "場所: $maintenanceTool" -ForegroundColor Red
    Write-Host ""
    Write-Host "Qt Online Installerから再インストールしてください:" -ForegroundColor Yellow
    Write-Host "https://www.qt.io/download-qt-installer" -ForegroundColor Cyan
    Read-Host "Enterキーで終了"
    exit 1
}

Write-Host "Qt MaintenanceTool found: $maintenanceTool" -ForegroundColor Green
Write-Host ""

# Check if Qt 6.6.3 is already installed
if (Test-Path $qt663Path) {
    Write-Host "Qt 6.6.3 は既にインストールされています！" -ForegroundColor Green
    Write-Host "場所: $qt663Path" -ForegroundColor Cyan
} else {
    Write-Host "Qt 6.6.3 をインストール中..." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "MaintenanceToolを起動します。" -ForegroundColor Cyan
    Write-Host "以下の手順を実行してください:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "1. MaintenanceToolが開いたら「コンポーネントを追加または削除」を選択" -ForegroundColor White
    Write-Host "2. 「Qt」→「Qt 6.6.3」を展開" -ForegroundColor White
    Write-Host "3. 「MSVC 2019 64-bit」にチェックを入れる" -ForegroundColor White
    Write-Host "4. 「次へ」→「更新」をクリック" -ForegroundColor White
    Write-Host "5. インストールが完了したらこのウィンドウに戻る" -ForegroundColor White
    Write-Host ""

    Read-Host "準備ができたらEnterキーを押してMaintenanceToolを起動"

    # Launch MaintenanceTool
    Start-Process $maintenanceTool

    Write-Host ""
    Write-Host "MaintenanceToolでインストールが完了したら..." -ForegroundColor Yellow
    Read-Host "Enterキーを押して続行"

    # Check again
    if (!(Test-Path $qt663Path)) {
        Write-Host ""
        Write-Host "エラー: Qt 6.6.3がインストールされていません" -ForegroundColor Red
        Write-Host "もう一度MaintenanceToolでインストールしてください" -ForegroundColor Yellow
        Read-Host "Enterキーで終了"
        exit 1
    }
}

Write-Host ""
Write-Host "====================================" -ForegroundColor Cyan
Write-Host "TLSプラグインをコピー中..." -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan

$tlsSource = "$qt663Path\plugins\tls"
$tlsDest = "$obsBinDir\tls"

# Check if TLS plugins exist
if (!(Test-Path $tlsSource)) {
    Write-Host ""
    Write-Host "エラー: Qt 6.6.3 TLSプラグインが見つかりません" -ForegroundColor Red
    Write-Host "場所: $tlsSource" -ForegroundColor Red
    Write-Host ""
    Write-Host "Qt 6.6.3のインストールが完全でない可能性があります" -ForegroundColor Yellow
    Read-Host "Enterキーで終了"
    exit 1
}

Write-Host "TLSプラグイン発見: $tlsSource" -ForegroundColor Green

# Create TLS directory in OBS
if (!(Test-Path $tlsDest)) {
    Write-Host "OBS TLSディレクトリを作成中..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $tlsDest -Force | Out-Null
}

# Remove old TLS plugins (Qt 6.8.3)
Write-Host "古いTLSプラグイン(Qt 6.8.3)を削除中..." -ForegroundColor Yellow
Remove-Item "$tlsDest\*.dll" -Force -ErrorAction SilentlyContinue

# Copy new TLS plugins (Qt 6.6.3)
Write-Host "新しいTLSプラグイン(Qt 6.6.3)をコピー中..." -ForegroundColor Yellow
Copy-Item "$tlsSource\*.dll" $tlsDest -Force

# Verify copy
$copiedFiles = Get-ChildItem -Path $tlsDest -Filter "*.dll"
if ($copiedFiles.Count -eq 0) {
    Write-Host ""
    Write-Host "エラー: TLSプラグインのコピーに失敗しました" -ForegroundColor Red
    Write-Host "管理者権限で実行してください" -ForegroundColor Yellow
    Read-Host "Enterキーで終了"
    exit 1
}

Write-Host ""
Write-Host "====================================" -ForegroundColor Green
Write-Host "インストール完了！" -ForegroundColor Green
Write-Host "====================================" -ForegroundColor Green
Write-Host ""
Write-Host "コピーされたTLSプラグイン:" -ForegroundColor Cyan
foreach ($file in $copiedFiles) {
    Write-Host "  - $($file.Name)" -ForegroundColor White
}
Write-Host ""
Write-Host "Qt Version: 6.6.3 (OBS 30.2.3と一致)" -ForegroundColor Green
Write-Host "保存先: $tlsDest" -ForegroundColor Cyan
Write-Host ""
Write-Host "====================================" -ForegroundColor Yellow
Write-Host "次のステップ:" -ForegroundColor Yellow
Write-Host "====================================" -ForegroundColor Yellow
Write-Host "1. OBS Studioを再起動" -ForegroundColor White
Write-Host "2. プラグイン設定を開く" -ForegroundColor White
Write-Host "3. 「モニタリング開始」をクリック" -ForegroundColor White
Write-Host "4. OBSログで「TLS initialization failed」エラーが" -ForegroundColor White
Write-Host "   出なくなっていることを確認" -ForegroundColor White
Write-Host ""

Read-Host "Enterキーで終了"
