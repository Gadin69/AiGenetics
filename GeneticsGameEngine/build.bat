@echo off

echo Building GeneticsGameEngine...

:: Check for Visual Studio 2022
if not exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.com" (
    if not exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.com" (
        if not exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\devenv.com" (
            echo ERROR: Visual Studio 2022 not found. Please install Visual Studio 2022 with C++ development tools.
            pause
            exit /b 1
        )
    )
)

echo Found Visual Studio 2022

echo Creating build directory...
mkdir build 2>nul

cd build

echo Configuring CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo Building project...
msbuild GeneticsGameEngine.sln /p:Configuration=Release /p:Platform=x64

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!
echo Starting application...

:: Change to bin directory and run the executable
cd ..\build\bin\Release
start GeneticsGameEngine.exe

echo Application started. Press any key to continue...
pause