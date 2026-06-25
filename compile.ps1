# PowerShell script to compile the game with Raylib and run it
$ErrorActionPreference = "Stop"

$includePath = (Resolve-Path "raylib_dep/raylib-5.0_win64_mingw-w64/include").Path
$libPath = (Resolve-Path "raylib_dep/raylib-5.0_win64_mingw-w64/lib").Path

if (-not (Test-Path "$includePath/raylib.h")) {
    Write-Error "Raylib headers not found at $includePath. Please run download_raylib.ps1 first!"
    exit 1
}

Write-Host "Compiling the game..."
# We use static linking flags for gcc runtime to ensure maximum portability
g++ src/main.cpp src/biom.cc -o main.exe `
    -I"$includePath" `
    -L"$libPath" `
    -lraylib `
    -lopengl32 `
    -lgdi32 `
    -lwinmm `
    -static-libgcc `
    -static-libstdc++ `
    -Wall `
    -O2

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful! Run .\main.exe to play."
} else {
    Write-Error "Compilation failed!"
}
