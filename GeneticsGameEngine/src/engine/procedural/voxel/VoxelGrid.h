#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>

namespace Engine {
namespace Procedural {
namespace Voxel {

// Voxel properties - From tables.txt TABLE 6 (lines 398-404)
struct Voxel {
    float density;          // 0.0 = empty, 1.0 = solid
    int materialType;       // Material category
    int colorIndex;         // Genetic color mapping
    float roughness;        // PBR parameter
    float metallic;         // PBR parameter
    
    Voxel() : density(0.0f), materialType(0), colorIndex(0), roughness(0.5f), metallic(0.0f) {}
};

// VoxelGrid data structure for creature generation
// From tables.txt TABLE 6 (lines 406-410)
class VoxelGrid {
private:
    std::vector<Voxel> m_data;          // Flattened 3D array [x + y*sizeX + z*sizeX*sizeY]
    std::vector<float> m_scalarField;   // Scalar field for marching cubes input
    int m_sizeX, m_sizeY, m_sizeZ;
    float m_voxelSize;                  // Physical size of each voxel

public:
    VoxelGrid();
    ~VoxelGrid();
    
    // Allocate grid with specified dimensions
    // Grid sizes from tables.txt TABLE 6: 32x32x32, 64x64x64, 128x128x128
    void AllocateGrid(int sizeX, int sizeY, int sizeZ, float voxelSize = 0.1f);
    
    // Convenience method for cubic grids
    void AllocateGrid(int size, float voxelSize = 0.1f);
    
    // Get voxel at position
    const Voxel& GetVoxel(int x, int y, int z) const;
    
    // Set voxel at position
    void SetVoxel(int x, int y, int z, const Voxel& voxel);
    
    // Get density at position (for marching cubes)
    float GetDensity(int x, int y, int z) const;
    
    // Set density at position (for marching cubes)
    void SetDensity(int x, int y, int z, float density);
    
    // Get scalar field value at position
    float GetScalarField(int x, int y, int z) const;
    
    // Set scalar field value at position
    void SetScalarField(int x, int y, int z, float value);
    
    // Get raw scalar field pointer (for GPU upload)
    const float* GetScalarFieldPointer() const;
    
    // Get grid dimensions
    int GetSizeX() const { return m_sizeX; }
    int GetSizeY() const { return m_sizeY; }
    int GetSizeZ() const { return m_sizeZ; }
    int GetTotalVoxels() const { return m_sizeX * m_sizeY * m_sizeZ; }
    
    // Get voxel size
    float GetVoxelSize() const { return m_voxelSize; }
    
    // Check if position is within bounds
    bool IsValidPosition(int x, int y, int z) const;
    
    // Clear grid (set all densities to 0)
    void Clear();
    
    // Get total size in bytes (for GPU buffer allocation)
    size_t GetScalarFieldSizeBytes() const;
    size_t GetVoxelDataSizeBytes() const;
};

} // namespace Voxel
} // namespace Procedural
} // namespace Engine
