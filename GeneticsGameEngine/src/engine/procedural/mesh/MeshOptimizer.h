#pragma once

#include <vector>
#include <cstring>
#include <DirectXMath.h>
#include "MarchingCubes.h"

namespace Engine {
namespace Procedural {
namespace Mesh {

// Quadric Error Metrics mesh simplification
// From tables.txt TABLE 10-11 (Garland & Heckbert 1997)
class MeshOptimizer {
public:
    MeshOptimizer() = default;
    
    // Simplify mesh to target triangle count using QEM
    // From tables.txt TABLE 10 (lines 568-595)
    MeshData SimplifyMesh(const MeshData& input, uint32_t targetTriangleCount) const;
    
private:
    // 4x4 symmetric quadric matrix
    struct Quadric {
        float m[10]; // Upper triangle of symmetric 4x4 matrix
        
        Quadric() { memset(m, 0, sizeof(m)); }
        
        static Quadric FromPlane(float a, float b, float c, float d);
        Quadric operator+(const Quadric& other) const;
        float Error(const DirectX::XMFLOAT3& v) const;
    };
    
    struct Edge {
        uint32_t v0, v1;
        double cost;
        DirectX::XMFLOAT3 optimalPosition;
    };
    
    // Compute quadrics for all vertices
    std::vector<Quadric> ComputeVertexQuadrics(const MeshData& mesh) const;
    
    // Find optimal collapse position for edge
    DirectX::XMFLOAT3 FindOptimalCollapsePosition(const Quadric& q, uint32_t v0, uint32_t v1, 
                                                   const MeshData& mesh) const;
};

} // namespace Mesh
} // namespace Procedural
} // namespace Engine
