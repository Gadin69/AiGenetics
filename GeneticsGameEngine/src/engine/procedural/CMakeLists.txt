# Procedural generation subsystem CMake configuration

# Voxel module
set(VOXEL_SOURCES
    voxel/VoxelGrid.cpp
)

set(VOXEL_HEADERS
    voxel/VoxelGrid.h
    voxel/VoxelLODManager.h
)

# Generation module
set(GENERATION_SOURCES
    generation/GeneticMapper.cpp
    generation/ScalarFieldGenerator.cpp
)

set(GENERATION_HEADERS
    generation/CreatureParams.h
    generation/GeneticMapper.h
    generation/ScalarFieldGenerator.h
)

# Mesh module
set(MESH_SOURCES
    mesh/MarchingCubes.cpp
    mesh/MeshOptimizer.cpp
    mesh/ProceduralMeshRenderer.cpp
)

set(MESH_HEADERS
    mesh/MarchingCubes.h
    mesh/MeshOptimizer.h
    mesh/ProceduralMeshRenderer.h
)

# Create procedural library
add_library(Procedural STATIC
    ${VOXEL_SOURCES}
    ${VOXEL_HEADERS}
    ${GENERATION_SOURCES}
    ${GENERATION_HEADERS}
    ${MESH_SOURCES}
    ${MESH_HEADERS}
)

# Set MSVC runtime library
if(MSVC)
    set_target_properties(Procedural PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()

# Set compile options
set_target_properties(Procedural PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Add /FS flag for PDB file sharing (MSVC parallel compilation fix)
# Must be applied via target_compile_options, not CMAKE_CXX_FLAGS
if(MSVC)
    target_compile_options(Procedural PRIVATE "/FS")
endif()

# Include directories
target_include_directories(Procedural PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add compile definitions for Windows
target_compile_definitions(Procedural PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)

# Link dependencies
target_link_libraries(Procedural PRIVATE
    d3d12
    dxgi
    dxguid
)
