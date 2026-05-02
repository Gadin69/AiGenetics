#pragma once

#include <vector>
#include <DirectXMath.h>
#include "../voxel/VoxelGrid.h"

namespace Engine {
namespace Procedural {
namespace Voxel {
    class VoxelGrid;  // Forward declaration
} // namespace Voxel

namespace Mesh {

// Output mesh data from marching cubes
struct MeshData {
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<DirectX::XMFLOAT3> normals;
    std::vector<uint32_t> indices;
};

// Marching Cubes algorithm for isosurface extraction
// All lookup tables copied VERBATIM from tables.txt - DO NOT MODIFY
class MarchingCubes {
public:
    MarchingCubes();
    
    // Generate mesh from voxel grid scalar field
    // isovalue: threshold for surface extraction (typically 0.0 or 0.5)
    MeshData GenerateMesh(const Voxel::VoxelGrid& grid, float isovalue = 0.0f) const;
    
private:
    // Vertex interpolation to find exact intersection point on edge
    // From tables.txt TABLE 4 (lines 309-323)
    DirectX::XMFLOAT3 VertexInterp(float isolevel,
                                   const DirectX::XMFLOAT3& p1,
                                   const DirectX::XMFLOAT3& p2,
                                   float valp1,
                                   float valp2) const;
    
    // Calculate surface normal at grid vertex using central differences gradient
    // From tables.txt TABLE 5 (lines 347-373)
    DirectX::XMFLOAT3 CalculateNormal(int x, int y, int z, const Voxel::VoxelGrid& grid) const;
    
    // Helper to calculate normal at interpolated vertex position
    DirectX::XMFLOAT3 CalculateNormalAtVertex(const DirectX::XMFLOAT3& vertex,
                                              const Voxel::VoxelGrid& grid,
                                              float isovalue) const;
    
    // ============================================================================
    // LOOKUP TABLES - COPIED VERBATIM FROM tables.txt
    // ⚠️  DO NOT MODIFY THESE TABLES UNDER ANY CIRCUMSTANCES ⚠️
    // Source: Paul Bourke - http://paulbourke.net/geometry/polygonise/
    // ============================================================================
    
    // Edge table maps cube configuration index (0-255) to 12-bit edge mask
    // From tables.txt TABLE 1 (lines 19-52)
    // ⚠️  DO NOT MODIFY - COPY-PASTED EXACTLY FROM tables.txt ⚠️
    static const int edgeTable[256];
    
    // Triangle table maps cube configuration index to triangle vertex indices
    // From tables.txt TABLE 2 (lines 67-250)
    // ⚠️  DO NOT MODIFY - COPY-PASTED EXACTLY FROM tables.txt ⚠️
    static const int triTable[256][16];
};

} // namespace Mesh
} // namespace Procedural
} // namespace Engine
