# Qt 6.6.3 TLS Plugins Installer
Write-Host "====================================" -ForegroundColor Cyan
Write-Host "Qt 6.6.3 TLS Plugin Downloader" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

$qtVersion = "6.6.3"
$qtUrl = "https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_663/qt.qt6.663.win64_msvc2019_64/"
$tempDir = "c:\Pokonyan-OBSObstruction-Plugin\qt663-temp"
$obsBinDir = "C:\Program Files\obs-studio\bin\64bit"

Write-Host "OBS 30.2.3 uses Qt 6.6.3, but we installed Qt 6.8.3 TLS plugins." -ForegroundColor Yellow
Write-Host "We need to download Qt 6.6.3 TLS plugins instead." -ForegroundColor Yellow
Write-Host ""
Write-Host "Please download Qt 6.6.3 manually:" -ForegroundColor Green
Write-Host ""
Write-Host "1. Go to: https://www.qt.io/download-qt-installer" -ForegroundColor Cyan
Write-Host "2. Download Qt Online Installer" -ForegroundColor Cyan
Write-Host "3. Install Qt 6.6.3 MSVC 2019 64-bit" -ForegroundColor Cyan
Write-Host "4. After installation, TLS plugins will be at:" -ForegroundColor Cyan
Write-Host "   C:\Qt\6.6.3\msvc2019_64\plugins\tls\" -ForegroundColor Yellow
Write-Host ""
Write-Host "Then run: install-qt663-tls.bat" -ForegroundColor Green
Write-Host ""

Read-Host "Press Enter to open Qt download page"
Start-Process "https://www.qt.io/download-qt-installer"
