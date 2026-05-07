// Production-grade Marching Cubes implementation
// With edge-based vertex caching, canonical ordering, interpolated normals

#include "MarchingCubes.h"
#include "VoxelGrid.h"
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <iostream>

namespace Engine {
namespace Procedural {
namespace Mesh {

// Edge vertex cache key (topology-based, not position-based)
struct EdgeKey
{
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

// Corner offsets for cube vertices
static constexpr int cornerOffsets[8][3] =
{
    {0,0,0},
    {1,0,0},
    {1,1,0},
    {0,1,0},
    {0,0,1},
    {1,0,1},
    {1,1,1},
    {0,1,1}
};

// Edge corner pairs (which two corners define each edge)
static constexpr int edgeCorners[12][2] =
{
    {0,1},
    {1,2},
    {2,3},
    {3,0},
    {4,5},
    {5,6},
    {6,7},
    {7,4},
    {0,4},
    {1,5},
    {2,6},
    {3,7}
};

// Sample gradient at grid position for normal calculation
DirectX::XMFLOAT3 SampleGradient(
    int x,
    int y,
    int z,
    const Voxel::VoxelGrid& grid)
{
    auto sample = [&](int sx, int sy, int sz)
    {
        sx = std::clamp(sx, 0, grid.GetSizeX() - 1);
        sy = std::clamp(sy, 0, grid.GetSizeY() - 1);
        sz = std::clamp(sz, 0, grid.GetSizeZ() - 1);

        return grid.GetScalarField(sx, sy, sz);
    };

    float dx = sample(x + 1,y,z) - sample(x - 1,y,z);
    float dy = sample(x,y + 1,z) - sample(x,y - 1,z);
    float dz = sample(x,y,z + 1) - sample(x,y,z - 1);

    DirectX::XMFLOAT3 n(-dx,-dy,-dz);

    float len = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);

    if (len > 0.00001f)
    {
        n.x /= len;
        n.y /= len;
        n.z /= len;
    }

    return n;
}

// Complete Marching Cubes Algorithm with edge-based vertex caching
MeshData MarchingCubes::GenerateMesh(const Voxel::VoxelGrid& grid, float isovalue) const {
    try {
        MeshData result;
        
        int sizeX = grid.GetSizeX();
        int sizeY = grid.GetSizeY();
        int sizeZ = grid.GetSizeZ();
        float voxelSize = grid.GetVoxelSize();
        
        std::cout << "      GenerateMesh: Grid size " << sizeX << "x" << sizeY << "x" << sizeZ << std::endl;
        
        // Calculate offset to center the creature
        float offsetX = -sizeX * voxelSize * 0.5f;
        float offsetY = -sizeY * voxelSize * 0.5f;
        float offsetZ = -sizeZ * voxelSize * 0.5f;
        
        // Edge-based vertex cache (local to this mesh generation)
        std::unordered_map<EdgeKey, uint32_t, EdgeKeyHasher> edgeVertexCache;
        
        int cubeCount = 0;
        int triangleCount = 0;
        
        for (int z = 0; z < sizeZ - 1; z++)
        {
            for (int y = 0; y < sizeY - 1; y++)
            {
                for (int x = 0; x < sizeX - 1; x++)
                {
                    cubeCount++;
                    
                    // Gather cube corner values, positions, and normals
                    float cubeValues[8];
                    DirectX::XMFLOAT3 cubePositions[8];
                    DirectX::XMFLOAT3 cubeNormals[8];

                    for (int i = 0; i < 8; i++)
                    {
                        int cx = x + cornerOffsets[i][0];
                        int cy = y + cornerOffsets[i][1];
                        int cz = z + cornerOffsets[i][2];

                        cubeValues[i] = grid.GetScalarField(cx, cy, cz);

                        cubePositions[i] =
                        {
                            cx * voxelSize + offsetX,
                            cy * voxelSize + offsetY,
                            cz * voxelSize + offsetZ
                        };

                        cubeNormals[i] = SampleGradient(cx, cy, cz, grid);
                    }

                    // Determine cube configuration index
                    int cubeIndex = 0;
                    for (int i = 0; i < 8; i++)
                    {
                        if (cubeValues[i] < isovalue)
                            cubeIndex |= (1 << i);
                    }

                    // Skip if cube is entirely inside or outside
                    if (edgeTable[cubeIndex] == 0)
                        continue;

                    // Calculate edge intersections and interpolated normals
                    DirectX::XMFLOAT3 edgeVertices[12];
                    DirectX::XMFLOAT3 edgeNormals[12];

                    for (int edge = 0; edge < 12; edge++)
                    {
                        if (!(edgeTable[cubeIndex] & (1 << edge)))
                            continue;

                        int c0 = edgeCorners[edge][0];
                        int c1 = edgeCorners[edge][1];

                        float v0 = cubeValues[c0];
                        float v1 = cubeValues[c1];

                        float t = (isovalue - v0) / (v1 - v0);

                        const auto& p0 = cubePositions[c0];
                        const auto& p1 = cubePositions[c1];

                        edgeVertices[edge] =
                        {
                            p0.x + t * (p1.x - p0.x),
                            p0.y + t * (p1.y - p0.y),
                            p0.z + t * (p1.z - p0.z)
                        };

                        const auto& n0 = cubeNormals[c0];
                        const auto& n1 = cubeNormals[c1];

                        edgeNormals[edge] =
                        {
                            n0.x + t * (n1.x - n0.x),
                            n0.y + t * (n1.y - n0.y),
                            n0.z + t * (n1.z - n0.z)
                        };

                        // Normalize interpolated normal
                        float nl = std::sqrt(
                            edgeNormals[edge].x * edgeNormals[edge].x +
                            edgeNormals[edge].y * edgeNormals[edge].y +
                            edgeNormals[edge].z * edgeNormals[edge].z);

                        if (nl > 0.00001f)
                        {
                            edgeNormals[edge].x /= nl;
                            edgeNormals[edge].y /= nl;
                            edgeNormals[edge].z /= nl;
                        }
                    }

                    // Assemble triangles from edge vertices
                    for (int i = 0; triTable[cubeIndex][i] != -1; i += 3)
                    {
                        uint32_t indices[3];

                        for (int v = 0; v < 3; v++)
                        {
                            int edge = triTable[cubeIndex][i + v];

                            EdgeKey key { x, y, z, edge };

                            auto it = edgeVertexCache.find(key);

                            if (it != edgeVertexCache.end())
                            {
                                indices[v] = it->second;
                            }
                            else
                            {
                                uint32_t index = (uint32_t)result.vertices.size();

                                result.vertices.push_back(edgeVertices[edge]);
                                result.normals.push_back(edgeNormals[edge]);

                                edgeVertexCache[key] = index;
                                indices[v] = index;
                            }
                        }

                        // Reject degenerate triangles
                        if (indices[0] == indices[1] ||
                            indices[1] == indices[2] ||
                            indices[2] == indices[0])
                        {
                            continue;
                        }

                        result.indices.push_back(indices[0]);
                        result.indices.push_back(indices[1]);
                        result.indices.push_back(indices[2]);
                        
                        triangleCount++;
                    }
                }
            }
            
            // Print progress every 16 slices
            if (z % 16 == 0) {
                std::cout << "      Progress: " << (z * 100 / (sizeZ - 1)) << "% - " 
                         << triangleCount << " triangles so far" << std::endl;
            }
        }
        
        std::cout << "      GenerateMesh complete: " << cubeCount << " cubes processed, " 
                 << triangleCount << " triangles generated" << std::endl;
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "      EXCEPTION in GenerateMesh: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "      UNKNOWN EXCEPTION in GenerateMesh!" << std::endl;
        throw;
    }
}

} // namespace Mesh
} // namespace Procedural
} // namespace Engine
