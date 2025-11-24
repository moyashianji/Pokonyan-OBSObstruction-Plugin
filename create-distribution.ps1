# Create distribution ZIP file
Write-Host "====================================" -ForegroundColor Cyan
Write-Host "YouTube SuperChat Plugin 配布ZIP作成" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

$distFolder = "c:\Pokonyan-OBSObstruction-Plugin\distribution"
$zipFile = "c:\Pokonyan-OBSObstruction-Plugin\YouTube-SuperChat-Plugin.zip"

# Check if distribution folder exists
if (!(Test-Path $distFolder)) {
    Write-Host "エラー: 配布フォルダが見つかりません" -ForegroundColor Red
    Write-Host "フォルダ: $distFolder" -ForegroundColor Red
    Read-Host "Enterキーで終了"
    exit 1
}

# Remove old ZIP if exists
if (Test-Path $zipFile) {
    Write-Host "既存のZIPファイルを削除中..." -ForegroundColor Yellow
    Remove-Item $zipFile -Force
}

# Create ZIP file
Write-Host "ZIPファイルを作成中..." -ForegroundColor Green
Compress-Archive -Path "$distFolder\*" -DestinationPath $zipFile -CompressionLevel Optimal

if (Test-Path $zipFile) {
    Write-Host ""
    Write-Host "====================================" -ForegroundColor Green
    Write-Host "ZIP作成完了！" -ForegroundColor Green
    Write-Host "====================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "ファイル: $zipFile" -ForegroundColor Cyan
    $fileSize = (Get-Item $zipFile).Length / 1KB
    Write-Host "サイズ: $([math]::Round($fileSize, 2)) KB" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "このZIPファイルを配布してください。" -ForegroundColor Yellow
    Write-Host "受け取った人は、解凍してINSTALL.batを実行するだけです。" -ForegroundColor Yellow
} else {
    Write-Host ""
    Write-Host "エラー: ZIP作成に失敗しました" -ForegroundColor Red
}

Write-Host ""
Read-Host "Enterキーで終了"
