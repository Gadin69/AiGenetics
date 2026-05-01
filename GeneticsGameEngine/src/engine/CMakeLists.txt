# Engine subsystem CMake configuration

# Add engine source files
set(ENGINE_SOURCES
)

# Add engine headers
set(ENGINE_HEADERS
    rendering/camera/CameraController.h
    rendering/camera/CameraSystem.h
    rendering/camera/CameraTypes.h
    rendering/camera/OrbitCameraController.h
    rendering/camera/FirstPersonCameraController.h
    rendering/camera/CinematicCameraController.h
    rendering/culling/BoundingVolume.h
    rendering/culling/FrustumCuller.h
    rendering/culling/SpatialPartition.h
    rendering/lod/LODManager.h
    rendering/lod/LODLevel.h
    rendering/projection/ProjectionMatrix.h
    rendering/projection/ProjectionModes.h
)

# Create engine library
if(${ENGINE_SOURCES})
    add_library(Engine STATIC
        ${ENGINE_SOURCES}
        ${ENGINE_HEADERS}
    )
    
    # Set MSVC runtime library for static library
    if(MSVC)
        set_target_properties(Engine PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
        )
    endif()
else()
    # Create interface library for header-only library
    add_library(Engine INTERFACE)
    target_include_directories(Engine INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_definitions(Engine INTERFACE
        WIN32
        _WINDOWS
        UNICODE
        _UNICODE
    )
endif()

# Set compile options
set_target_properties(Engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)