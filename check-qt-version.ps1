# Check Qt version mismatch
Write-Host "====================================" -ForegroundColor Cyan
Write-Host "Qt Version Checker" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

# Find OBS installation
$obsPath = "C:\Program Files\obs-studio"
if (Test-Path $obsPath) {
    Write-Host "OBS Studio found at: $obsPath" -ForegroundColor Green

    # Check for Qt DLLs
    $qt6CorePath = Join-Path $obsPath "bin\64bit\Qt6Core.dll"
    if (Test-Path $qt6CorePath) {
        $versionInfo = (Get-Item $qt6CorePath).VersionInfo
        Write-Host "OBS Qt Version: $($versionInfo.FileVersion)" -ForegroundColor Yellow
    }
} else {
    Write-Host "OBS Studio not found at default location" -ForegroundColor Red
}

Write-Host ""
Write-Host "Plugin was built with Qt: 6.10.1" -ForegroundColor Yellow
Write-Host ""
Write-Host "Solution: Rebuild plugin with OBS's Qt version (6.8.3)" -ForegroundColor Green
Write-Host ""

# Check if Qt 6.8.3 is available
$qt683Path = "C:\Qt\6.8.3\msvc2022_64"
if (Test-Path $qt683Path) {
    Write-Host "Qt 6.8.3 found at: $qt683Path" -ForegroundColor Green
    Write-Host "Run: build-with-obs-qt.ps1" -ForegroundColor Cyan
} else {
    Write-Host "Qt 6.8.3 not found. Please install Qt 6.8.3 from:" -ForegroundColor Red
    Write-Host "  https://www.qt.io/download-qt-installer" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "After installation, update build-final.ps1:" -ForegroundColor Yellow
    Write-Host '  $QT_PATH = "C:/Qt/6.8.3/msvc2022_64"' -ForegroundColor Cyan
}

Write-Host ""
Read-Host "Press Enter to exit"
