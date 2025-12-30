# Install VS 2022 Build Tools with C++ workload
$ErrorActionPreference = "Continue"

Write-Host "Installing Visual Studio 2022 Build Tools with C++ workload..." -ForegroundColor Green

# Run VS installer to add C++ workload
$installerPath = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"

if (Test-Path $installerPath) {
    Write-Host "VS Installer found, adding C++ workload..." -ForegroundColor Cyan
    & $installerPath modify `
        --installPath "C:\Program Files\Microsoft Visual Studio\2022\BuildTools" `
        --add Microsoft.VisualStudio.Workload.VCTools `
        --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        --add Microsoft.VisualStudio.Component.Windows11SDK.22621 `
        --includeRecommended `
        --passive `
        --wait

    Write-Host "Installation completed." -ForegroundColor Green
} else {
    Write-Host "VS Installer not found at $installerPath" -ForegroundColor Red
    Write-Host "Trying to use winget..." -ForegroundColor Yellow

    # Alternative: use winget with override
    winget install --id Microsoft.VisualStudio.2022.BuildTools `
        --override "--add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.22621 --includeRecommended --passive --wait" `
        --accept-source-agreements --accept-package-agreements
}

# Check if installation was successful
$msvcPath = "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC"
if (Test-Path $msvcPath) {
    Write-Host ""
    Write-Host "SUCCESS! MSVC tools installed at:" -ForegroundColor Green
    Get-ChildItem $msvcPath
} else {
    Write-Host ""
    Write-Host "MSVC tools not found. Installation may still be in progress." -ForegroundColor Yellow
}
