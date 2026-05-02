#include "VoxelGrid.h"
#include <stdexcept>

namespace Engine {
namespace Procedural {
namespace Voxel {

VoxelGrid::VoxelGrid() : m_sizeX(0), m_sizeY(0), m_sizeZ(0), m_voxelSize(0.1f) {
}

VoxelGrid::~VoxelGrid() {
    // Vectors clean up automatically
}

// Allocate grid with specified dimensions
// Grid sizes from tables.txt TABLE 6: 32x32x32, 64x64x64, 128x128x128
void VoxelGrid::AllocateGrid(int sizeX, int sizeY, int sizeZ, float voxelSize) {
    if (sizeX <= 0 || sizeY <= 0 || sizeZ <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
    
    m_sizeX = sizeX;
    m_sizeY = sizeY;
    m_sizeZ = sizeZ;
    m_voxelSize = voxelSize;
    
    // Allocate flattened 3D arrays
    size_t totalVoxels = static_cast<size_t>(sizeX) * sizeY * sizeZ;
    m_data.resize(totalVoxels);
    m_scalarField.resize(totalVoxels, 0.0f);
}

// Convenience method for cubic grids
void VoxelGrid::AllocateGrid(int size, float voxelSize) {
    AllocateGrid(size, size, size, voxelSize);
}

// Helper function to calculate flattened index
inline size_t CalculateIndex(int x, int y, int z, int sizeX, int sizeY) {
    return static_cast<size_t>(x) + y * sizeX + z * sizeX * sizeY;
}

const Voxel& VoxelGrid::GetVoxel(int x, int y, int z) const {
    if (!IsValidPosition(x, y, z)) {
        throw std::out_of_range("Voxel position out of bounds");
    }
    size_t index = CalculateIndex(x, y, z, m_sizeX, m_sizeY);
    return m_data[index];
}

void VoxelGrid::SetVoxel(int x, int y, int z, const Voxel& voxel) {
    if (!IsValidPosition(x, y, z)) {
        throw std::out_of_range("Voxel position out of bounds");
    }
    size_t index = CalculateIndex(x, y, z, m_sizeX, m_sizeY);
    m_data[index] = voxel;
}

float VoxelGrid::GetDensity(int x, int y, int z) const {
    if (!IsValidPosition(x, y, z)) {
        return 0.0f;
    }
    size_t index = CalculateIndex(x, y, z, m_sizeX, m_sizeY);
    return m_data[index].density;
}

void VoxelGrid::SetDensity(int x, int y, int z, float density) {
    if (!IsValidPosition(x, y, z)) {
        return;
    }
    size_t index = CalculateIndex(x, y, z, m_sizeX, m_sizeY);
    m_data[index].density = density;
}

float VoxelGrid::GetScalarField(int x, int y, int z) const {
    if (!IsValidPosition(x, y, z)) {
        return 0.0f;
    }
    size_t index = CalculateIndex(x, y, z, m_sizeX, m_sizeY);
    return m_scalarField[index];
}

void VoxelGrid::SetScalarField(int x, int y, int z, float value) {
    if (!IsValidPosition(x, y, z)) {
        return;
    }
    size_t index = CalculateIndex(x, y, z, m_sizeX, m_sizeY);
    m_scalarField[index] = value;
}

const float* VoxelGrid::GetScalarFieldPointer() const {
    return m_scalarField.data();
}

bool VoxelGrid::IsValidPosition(int x, int y, int z) const {
    return x >= 0 && x < m_sizeX && 
           y >= 0 && y < m_sizeY && 
           z >= 0 && z < m_sizeZ;
}

void VoxelGrid::Clear() {
    for (auto& voxel : m_data) {
        voxel.density = 0.0f;
    }
    std::fill(m_scalarField.begin(), m_scalarField.end(), 0.0f);
}

size_t VoxelGrid::GetScalarFieldSizeBytes() const {
    return m_scalarField.size() * sizeof(float);
}

size_t VoxelGrid::GetVoxelDataSizeBytes() const {
    return m_data.size() * sizeof(Voxel);
}

} // namespace Voxel
} // namespace Procedural
} // namespace Engine
