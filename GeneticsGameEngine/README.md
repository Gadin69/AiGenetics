# 3D Genetics Game Engine

## Overview

This is a DirectX 12-based 3D genetics game engine that combines ARK-style breeding with Dragon Adventures genetics. The engine features:

- OOP architecture with inheritance and polymorphism
- Biological taxonomy-based class hierarchy (Chordata, Arthropoda, Mollusca)
- Dynamic neural networks with hybrid growth triggers
- Multi-gene interaction systems (epistasis, polygenic traits, regulatory genes)
- Procedural mesh and movement generation synchronized through genetic data
- AAA performance optimizations (multi-threaded rendering, GPU memory management)

## What the Application Does When It Opens

When you run the executable (`GeneticsGameEngine.exe`), the application will:

1. **Open a window**: A 800x600 window titled "3D Genetics Game" will appear
2. **Render a test scene**: A colored triangle will be rendered in the window
3. **Stay open continuously**: The application will run indefinitely, processing Windows messages and rendering frames at 60 FPS
4. **Display debug information**: FPS counter will be printed to the console every second
5. **Show genetic integration**: A test creature "test_creature_001" from the "Chordata" class will be added to the genetics system

## Building the Project

### Prerequisites
- Visual Studio 2022 with C++ development tools
- Windows SDK (automatically installed with Visual Studio)
- CMake 3.25 or later

### Build Instructions

**Using build.bat (Windows Command Prompt):**
```cmd
build.bat
```

**Using build.ps1 (PowerShell):**
```powershell
.uild.ps1
```

**Manual build:**
```cmd
cd GeneticsGameEngine
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
msbuild GeneticsGameEngine.sln /p:Configuration=Release /p:Platform=x64
```

The executable will be located at `build\bin\Release\GeneticsGameEngine.exe`.

## Running the Application

After building, simply run:
```cmd
build\bin\Release\GeneticsGameEngine.exe
```

The application will open a window and render the 3D scene continuously.