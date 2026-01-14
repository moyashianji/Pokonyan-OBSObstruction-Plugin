# Generate .lib files from OBS DLLs using gendef and dlltool approach
$ErrorActionPreference = "Continue"

Write-Host "===================================" -ForegroundColor Green
Write-Host "Generating .lib files from OBS DLLs" -ForegroundColor Green
Write-Host "===================================" -ForegroundColor Green

# Visual Studio paths
$vsPath = "C:\Program Files\Microsoft Visual Studio\18\Insiders"
$msvcPath = "$vsPath\VC\Tools\MSVC\14.50.35717"
$sdkPath = "C:\Program Files (x86)\Windows Kits\10"
$sdkVersion = "10.0.22621.0"

# Set environment
$env:PATH = "$sdkPath\bin\$sdkVersion\x64;$msvcPath\bin\Hostx64\x64;$env:PATH"

$obsDir = "C:\Program Files\obs-studio\bin\64bit"
$outputDir = "C:\obs-libs"

$dumpbinPath = "$msvcPath\bin\Hostx64\x64\dumpbin.exe"
$libPath = "$msvcPath\bin\Hostx64\x64\lib.exe"

# Function to extract all exports from DLL
function Get-DllExports {
    param($dllPath, $libraryName)

    Write-Host "Extracting exports from $dllPath..." -ForegroundColor Cyan
    $output = & $dumpbinPath /EXPORTS $dllPath 2>&1

    $exports = @()
    $capturing = $false

    foreach ($line in $output) {
        $lineStr = $line.ToString()

        # Start capturing after the header line
        if ($lineStr -match "ordinal\s+hint\s+RVA\s+name") {
            $capturing = $true
            continue
        }

        # Stop at summary section
        if ($capturing -and $lineStr -match "^\s*Summary") {
            break
        }

        # Parse export lines
        if ($capturing -and $lineStr -match "^\s+(\d+)\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+(\S+)") {
            $ordinal = $matches[1]
            $name = $matches[2]
            $exports += $name
        }
    }

    Write-Host "Found $($exports.Count) exports" -ForegroundColor Yellow
    return $exports
}

# Generate obs.lib
Write-Host ""
Write-Host "=== Processing obs.dll ===" -ForegroundColor Green
$obsExports = Get-DllExports "$obsDir\obs.dll" "obs"

$defContent = "LIBRARY obs.dll`r`nEXPORTS`r`n"
foreach ($export in $obsExports) {
    $defContent += "    $export`r`n"
}
[System.IO.File]::WriteAllText("$outputDir\obs.def", $defContent)

Write-Host "Creating obs.lib from def file..." -ForegroundColor Yellow
$result = & $libPath /DEF:"$outputDir\obs.def" /OUT:"$outputDir\obs.lib" /MACHINE:X64 2>&1
Write-Host $result

# Generate obs-frontend-api.lib
Write-Host ""
Write-Host "=== Processing obs-frontend-api.dll ===" -ForegroundColor Green
$frontendExports = Get-DllExports "$obsDir\obs-frontend-api.dll" "obs-frontend-api"

$defContent = "LIBRARY obs-frontend-api.dll`r`nEXPORTS`r`n"
foreach ($export in $frontendExports) {
    $defContent += "    $export`r`n"
}
[System.IO.File]::WriteAllText("$outputDir\obs-frontend-api.def", $defContent)

Write-Host "Creating obs-frontend-api.lib from def file..." -ForegroundColor Yellow
$result = & $libPath /DEF:"$outputDir\obs-frontend-api.def" /OUT:"$outputDir\obs-frontend-api.lib" /MACHINE:X64 2>&1
Write-Host $result

# Show results
Write-Host ""
Write-Host "=== Generated files ===" -ForegroundColor Green
Get-ChildItem $outputDir | Format-Table Name, Length

# Show def file contents for verification
Write-Host ""
Write-Host "obs.def exports (first 20):" -ForegroundColor Cyan
Get-Content "$outputDir\obs.def" | Select-Object -First 25

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
