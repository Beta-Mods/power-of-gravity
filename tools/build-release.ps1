[CmdletBinding()]
param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$RepositoryRoot = Split-Path -Parent $PSScriptRoot
Set-Location $RepositoryRoot

if (-not $env:VCPKG_ROOT) {
    throw "VCPKG_ROOT is not set. Point it at a bootstrapped vcpkg checkout."
}

if (-not (Test-Path (Join-Path $env:VCPKG_ROOT "scripts/buildsystems/vcpkg.cmake"))) {
    throw "VCPKG_ROOT does not contain scripts/buildsystems/vcpkg.cmake: $env:VCPKG_ROOT"
}

if (-not $env:VCPKG_VISUAL_STUDIO_PATH) {
    $vs2022Community = 'C:\Program Files\Microsoft Visual Studio\2022\Community'
    $vs2022BuildTools = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools'
    if (Test-Path (Join-Path $vs2022Community 'VC\Auxiliary\Build\vcvarsall.bat')) {
        $env:VCPKG_VISUAL_STUDIO_PATH = $vs2022Community
    } elseif (Test-Path (Join-Path $vs2022BuildTools 'VC\Auxiliary\Build\vcvarsall.bat')) {
        $env:VCPKG_VISUAL_STUDIO_PATH = $vs2022BuildTools
    }
}

if (-not $env:VSCMD_VER) {
    Write-Warning "Not running inside a Visual Studio developer environment. If vcpkg triplet validation fails, run from 'x64 Native Tools Command Prompt for VS 2022' or invoke through vcvarsall.bat."
}

$BuildDirectory = Join-Path $RepositoryRoot "build/vs2022"
if ($Clean -and (Test-Path $BuildDirectory)) {
    Remove-Item $BuildDirectory -Recurse -Force
}

Write-Host "Configuring Custom Moon Sizes 2..."
cmake --preset vs2022
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

Write-Host "Building Release..."
cmake --build --preset vs2022-release --parallel
if ($LASTEXITCODE -ne 0) { throw "Release build failed." }

Write-Host "Running tests..."
ctest --preset vs2022-release
if ($LASTEXITCODE -ne 0) { throw "Tests failed." }

Write-Host "Creating release ZIP..."
cmake --build $BuildDirectory --config Release --target package
if ($LASTEXITCODE -ne 0) { throw "Packaging failed." }

$Packages = Get-ChildItem $BuildDirectory -Filter "CustomMoonSizes2-*.zip" | Sort-Object LastWriteTime -Descending
if (-not $Packages) {
    throw "The package target completed but no ZIP was found in $BuildDirectory."
}

Write-Host "Release package: $($Packages[0].FullName)"
Write-Host "Loose files: $(Join-Path $BuildDirectory 'dist')"
