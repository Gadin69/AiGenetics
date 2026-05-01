# Genetics Game Engine Build Script

Write-Host "Building GeneticsGameEngine..." -ForegroundColor Green

# Check for Visual Studio 2022
$vsPaths = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.com",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.com",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\devenv.com"
)

$vsFound = $false
foreach ($path in $vsPaths) {
    if (Test-Path $path) {
        Write-Host "✓ Found Visual Studio 2022 at $path"
        $vsFound = $true
        break
    }
}

if (-not $vsFound) {
    Write-Error "ERROR: Visual Studio 2022 not found. Please install Visual Studio 2022 with C++ development tools."
    Pause
    Exit 1
}

# Create build directory
Write-Host "Creating build directory..."
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Configure CMake
Write-Host "Configuring CMake..."
Set-Location "build"
$cmakeResult = cmake -G "Visual Studio 17 2022" -A x64 ..
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed!"
    Pause
    Exit 1
}

# Build the project
Write-Host "Building project..."
$buildResult = msbuild GeneticsGameEngine.sln /p:Configuration=Release /p:Platform=x64
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    Pause
    Exit 1
}

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Starting application..."

# Change to bin directory and run the executable
Set-Location "..\build\bin\Release"
Start-Process "GeneticsGameEngine.exe"

Write-Host "Application started. Press any key to continue..."
$host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown") | Out-Null