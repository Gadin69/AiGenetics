@echo off

REM 3D Genetics Game Engine Run Script
REM This script builds and runs the Genetics Game Engine

setlocal

ECHO =====================================
ECHO 3D Genetics Game Engine Launcher
ECHO =====================================

REM Change to GeneticsGameEngine directory
CD /D "%~dp0"

REM Check if build directory exists
IF NOT EXIST "build" (
    ECHO Build directory not found. Creating...
    mkdir build
)

REM Change to build directory
CD build

REM Check for Visual Studio 2022 Community installation
ECHO Checking for Visual Studio 2022 Community...
IF NOT EXIST "C:\Program Files\Microsoft Visual Studio\2022\Community" (
    ECHO ERROR: Visual Studio 2022 Community not found at default location.
    ECHO Please install Visual Studio 2022 Community with C++ development tools.
    PAUSE
    EXIT /B 1
)

REM Check for CMake installation
ECHO Checking for CMake...
cmake --version >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: CMake not found. Please install CMake from https://cmake.org/download/
    PAUSE
    EXIT /B 1
)

REM Generate Visual Studio solution if it doesn't exist
IF NOT EXIST "GeneticsGameEngine.sln" (
    ECHO Generating Visual Studio solution...
    cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
    IF %ERRORLEVEL% NEQ 0 (
        ECHO ERROR: Failed to generate Visual Studio solution.
        PAUSE
        EXIT /B 1
    )
)

REM Build the project using CMake (more reliable than MSBuild)
ECHO Building GeneticsGameEngine...
cmake --build . --config Release --target GeneticsGameEngine
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: Build failed.
    PAUSE
    EXIT /B 1
)

REM Check if executable was created
IF NOT EXIST "Release\GeneticsGameEngine.exe" (
    ECHO ERROR: Executable not found after build.
    PAUSE
    EXIT /B 1
)

ECHO Build successful!

REM Run the executable
ECHO Starting 3D Genetics Game Engine...
START "" "Release\GeneticsGameEngine.exe"

ECHO Game engine launched successfully!
PAUSE