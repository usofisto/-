# PowerShell script to download and extract Raylib 5.0 for Windows MinGW
$ErrorActionPreference = "Stop"

$dir = "raylib_dep"
if (-not (Test-Path $dir)) {
    Write-Host "Creating directory $dir..."
    New-Item -ItemType Directory -Path $dir | Out-Null
}

$zipPath = "$dir\raylib.zip"
$url = "https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_win64_mingw-w64.zip"

Write-Host "Downloading Raylib 5.0 from $url..."
Invoke-WebRequest -Uri $url -OutFile $zipPath

Write-Host "Extracting Raylib..."
Expand-Archive -Path $zipPath -DestinationPath $dir -Force

Write-Host "Cleaning up zip file..."
Remove-Item -Path $zipPath

Write-Host "Raylib setup completed successfully!"
