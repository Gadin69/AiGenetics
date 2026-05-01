@echo off

echo =====================================
echo 3D Genetics Game Engine Build Script
echo =====================================

:: Check if Visual Studio 2022 is installed
echo Checking for Visual Studio 2022...
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" (
    echo ✓ Visual Studio 2022 found
) else (
    echo ✗ Visual Studio 2022 not found. Please install Visual Studio 2022 with C++ development tools.
    pause
    exit /b 1
)

:: Create build directory
if not exist "build" mkdir "build"

echo Creating build directory...

:: Change to build directory
cd "build"

echo Running CMake to generate Visual Studio solution...
:: Run CMake to generate Visual Studio solution
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo ✗ CMake configuration failed
    pause
    exit /b 1
) else (
    echo ✓ CMake configuration successful
)

:: Build the solution
echo Building GeneticsGameEngine...
cmake --build . --config Release --target GeneticsGameEngine
if %errorlevel% neq 0 (
    echo ✗ Build failed
    pause
    exit /b 1
) else (
    echo ✓ Build successful
)

:: Check if executable was created
echo Checking for executable...
if exist "Release\GeneticsGameEngine.exe" (
    echo ✓ Executable found: Release\GeneticsGameEngine.exe
    
    :: Run the executable
    echo Starting Genetics Game Engine...
    echo (The 3D window will appear shortly)
    
    :: Start the executable and wait for it to finish
    start "" /wait "Release\GeneticsGameEngine.exe"
    
    echo Game engine closed.
) else (
    echo ✗ Executable not found. Build may have failed.
    pause
    exit /b 1
)

echo =====================================
echo Build and launch completed!
echo =====================================
pause