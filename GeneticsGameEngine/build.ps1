# PowerShell Build Script for Genetics Game Engine

Write-Host "=====================================" -ForegroundColor Green
Write-Host "3D Genetics Game Engine Build Script" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green

# Check if Visual Studio 2022 is installed
Write-Host "Checking for Visual Studio 2022..." -ForegroundColor Yellow
$vs2022 = Get-ChildItem "HKLM:\SOFTWARE\Microsoft\DevDiv\VS\Servicing\*\community" -ErrorAction SilentlyContinue | Where-Object { $_.Name -match "17.0" }
if ($vs2022) {
    Write-Host "✓ Visual Studio 2022 found" -ForegroundColor Green
} else {
    Write-Host "✗ Visual Studio 2022 not found. Please install Visual Studio 2022 with C++ development tools." -ForegroundColor Red
    exit 1
}

# Create build directory
Write-Host "Creating build directory..." -ForegroundColor Yellow
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Change to build directory
Set-Location "build"

# Run CMake to generate Visual Studio solution
Write-Host "Running CMake to generate Visual Studio solution..." -ForegroundColor Yellow
try {
    cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ CMake configuration successful" -ForegroundColor Green
    } else {
        Write-Host "✗ CMake configuration failed" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "✗ CMake execution failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Build the solution
Write-Host "Building GeneticsGameEngine..." -ForegroundColor Yellow
try {
    cmake --build . --config Release --target GeneticsGameEngine
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Build successful" -ForegroundColor Green
    } else {
        Write-Host "✗ Build failed" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "✗ Build execution failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Check if executable was created
Write-Host "Checking for executable..." -ForegroundColor Yellow
if (Test-Path "Release\GeneticsGameEngine.exe") {
    Write-Host "✓ Executable found: Release\GeneticsGameEngine.exe" -ForegroundColor Green
    
    # Run the executable
    Write-Host "Starting Genetics Game Engine..." -ForegroundColor Yellow
    Write-Host "(The 3D window will appear shortly)" -ForegroundColor Cyan
    
    # Start the executable and wait for it to finish
    Start-Process -FilePath "Release\GeneticsGameEngine.exe" -Wait
    
    Write-Host "Game engine closed." -ForegroundColor Green
} else {
    Write-Host "✗ Executable not found. Build may have failed." -ForegroundColor Red
    exit 1
}

Write-Host "=====================================" -ForegroundColor Green
Write-Host "Build and launch completed!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green