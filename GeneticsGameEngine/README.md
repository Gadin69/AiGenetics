# 3D Genetics Game Engine

A high-performance DirectX 12 engine for the 3D Genetics Game with AAA optimizations focused on framerate.

## Features

- **DirectX 12 Core**: Low-overhead graphics API for maximum performance
- **60 FPS Target**: Optimized for stable 60 FPS at 1080p resolution
- **Genetics Integration**: Seamless connection to your existing genetics framework
- **AAA Optimizations**: Multi-threaded rendering, GPU memory management, and synchronization primitives
- **Modular Architecture**: Clean separation of concerns with core, graphics, and genetics subsystems

## Build Requirements

- Visual Studio 2022 (C++ development tools)
- Windows SDK 10.0.22621.0 or later
- DirectX 12 Graphics SDK
- CMake 3.25+

## Building

1. Open Visual Studio 2022
2. Load the `GeneticsGameEngine.sln` solution file
3. Build the solution in Release configuration
4. Run the executable

## Project Structure

```
/GeneticsGameEngine/
├── /build/                # Build artifacts
├── /src/
│   ├── /core/            # Engine core (DX12, memory management)
│   ├── /graphics/        # DX12 rendering pipeline
│   ├── /genetics/        # Your existing genetics framework integration
│   ├── /physics/         # Physics simulation
│   └── /audio/           # Audio system
├── /assets/              # 3D models, textures, shaders
├── /shaders/             # HLSL shader files
├── CMakeLists.txt
└── GeneticsGameEngine.sln
```

## Performance Monitoring

The engine includes built-in FPS counter that prints to console every second.

## Next Steps

- Add physics simulation for realistic creature movement
- Implement audio system for environmental sounds
- Add advanced rendering features (ray tracing, volumetric lighting)
- Integrate with your existing genetics framework for full functionality