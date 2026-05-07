#pragma once

#include <vector>
#include <DirectXMath.h>
#include <unordered_map>
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

// Edge key for topology-based vertex caching
struct EdgeKey {
    int x;
    int y;
    int z;
    int edge;

    bool operator==(const EdgeKey& other) const
    {
        return x == other.x &&
               y == other.y &&
               z == other.z &&
               edge == other.edge;
    }
};

struct EdgeKeyHasher
{
    size_t operator()(const EdgeKey& k) const
    {
        return
            ((size_t)k.x * 73856093) ^
            ((size_t)k.y * 19349663) ^
            ((size_t)k.z * 83492791) ^
            ((size_t)k.edge * 2654435761);
    }
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
    
    // Sample gradient at grid position using central differences
    DirectX::XMFLOAT3 SampleGradient(int x, int y, int z, const Voxel::VoxelGrid& grid) const;
    
    // ============================================================================
    // LOOKUP TABLES - COPIED VERBATIM FROM tables.txt
    // ⚠️  DO NOT MODIFY THESE TABLES UNDER ANY CIRCUMSTANCES ⚠️
    // Source: Paul Bourke - http://paulbourke.net/geometry/polygonise/
    // ============================================================================
    
    // Canonical corner offsets for cube vertices (0-7)
    static constexpr int cornerOffsets[8][3] =
    {
        {0,0,0},  // 0
        {1,0,0},  // 1
        {1,1,0},  // 2
        {0,1,0},  // 3
        {0,0,1},  // 4
        {1,0,1},  // 5
        {1,1,1},  // 6
        {0,1,1}   // 7
    };
    
    // Canonical edge corner pairs (12 edges, 2 corners each)
    static constexpr int edgeCorners[12][2] =
    {
        {0,1},  // 0
        {1,2},  // 1
        {2,3},  // 2
        {3,0},  // 3
        {4,5},  // 4
        {5,6},  // 5
        {6,7},  // 6
        {7,4},  // 7
        {0,4},  // 8
        {1,5},  // 9
        {2,6},  // 10
        {3,7}   // 11
    };
    
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
