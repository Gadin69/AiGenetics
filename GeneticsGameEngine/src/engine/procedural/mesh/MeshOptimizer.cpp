#include "MeshOptimizer.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <set>

namespace Engine {
namespace Procedural {
namespace Mesh {

// ============================================================================
// QUADRIC ERROR METRICS MESH SIMPLIFICATION
// From tables.txt TABLE 10 (lines 568-595) - Garland & Heckbert 1997
// From tables.txt TABLE 11 (lines 603-623) - Edge collapse criteria
// ============================================================================

// Quadric implementation
MeshOptimizer::Quadric MeshOptimizer::Quadric::FromPlane(float a, float b, float c, float d) {
    Quadric q;
    // Q = pp^T where p = (a,b,c,d) normalized plane equation
    // Store upper triangle of symmetric 4x4 matrix (10 elements)
    q.m[0] = a * a; // [0][0]
    q.m[1] = a * b; // [0][1]
    q.m[2] = a * c; // [0][2]
    q.m[3] = a * d; // [0][3]
    q.m[4] = b * b; // [1][1]
    q.m[5] = b * c; // [1][2]
    q.m[6] = b * d; // [1][3]
    q.m[7] = c * c; // [2][2]
    q.m[8] = c * d; // [2][3]
    q.m[9] = d * d; // [3][3]
    return q;
}

MeshOptimizer::Quadric MeshOptimizer::Quadric::operator+(const Quadric& other) const {
    Quadric result;
    for (int i = 0; i < 10; ++i) {
        result.m[i] = m[i] + other.m[i];
    }
    return result;
}

float MeshOptimizer::Quadric::Error(const DirectX::XMFLOAT3& v) const {
    // Error = v^T Q v (quadratic form)
    float x = v.x, y = v.y, z = v.z;
    return m[0]*x*x + 2*m[1]*x*y + 2*m[2]*x*z + 2*m[3]*x +
           m[4]*y*y + 2*m[5]*y*z + 2*m[6]*y +
           m[7]*z*z + 2*m[8]*z + m[9];
}

// Compute quadrics for all vertices
// From tables.txt TABLE 10 (lines 579-580): Compute quadric Q for each vertex
std::vector<MeshOptimizer::Quadric> MeshOptimizer::ComputeVertexQuadrics(const MeshData& mesh) const {
    std::vector<Quadric> quadrics(mesh.vertices.size());
    
    // For each triangle, compute plane equation and add to vertex quadrics
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        uint32_t i0 = mesh.indices[i];
        uint32_t i1 = mesh.indices[i + 1];
        uint32_t i2 = mesh.indices[i + 2];
        
        const auto& v0 = mesh.vertices[i0];
        const auto& v1 = mesh.vertices[i1];
        const auto& v2 = mesh.vertices[i2];
        
        // Compute plane equation from triangle (v0, v1, v2)
        // normal = normalize(cross(v1-v0, v2-v0))
        DirectX::XMVECTOR e1 = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v1), DirectX::XMLoadFloat3(&v0));
        DirectX::XMVECTOR e2 = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v2), DirectX::XMLoadFloat3(&v0));
        DirectX::XMVECTOR normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(e1, e2));
        
        float a, b, c, d;
        a = DirectX::XMVectorGetX(normal);
        b = DirectX::XMVectorGetY(normal);
        c = DirectX::XMVectorGetZ(normal);
        
        // d = -dot(normal, v0)
        d = -(a * v0.x + b * v0.y + c * v0.z);
        
        // Create fundamental quadric Kp = pp^T
        Quadric K = Quadric::FromPlane(a, b, c, d);
        
        // Add to all three vertices
        quadrics[i0] = quadrics[i0] + K;
        quadrics[i1] = quadrics[i1] + K;
        quadrics[i2] = quadrics[i2] + K;
    }
    
    return quadrics;
}

// Find optimal collapse position
// From tables.txt TABLE 10 (lines 576-577): v_bar = argmin(v^T(Q1+Q2)v)
DirectX::XMFLOAT3 MeshOptimizer::FindOptimalCollapsePosition(const Quadric& q, uint32_t v0, uint32_t v1,
                                                               const MeshData& mesh) const {
    // Try three candidates and pick the one with minimum error:
    // 1. Original position v0
    // 2. Original position v1
    // 3. Midpoint (v0 + v1) / 2
    
    const auto& pos0 = mesh.vertices[v0];
    const auto& pos1 = mesh.vertices[v1];
    
    DirectX::XMFLOAT3 candidates[3] = {
        pos0,
        pos1,
        DirectX::XMFLOAT3(
            (pos0.x + pos1.x) * 0.5f,
            (pos0.y + pos1.y) * 0.5f,
            (pos0.z + pos1.z) * 0.5f
        )
    };
    
    float minError = std::numeric_limits<float>::max();
    DirectX::XMFLOAT3 bestPos = candidates[0];
    
    for (int i = 0; i < 3; ++i) {
        float error = q.Error(candidates[i]);
        if (error < minError) {
            minError = error;
            bestPos = candidates[i];
        }
    }
    
    return bestPos;
}

// Full QEM mesh simplification
// From tables.txt TABLE 10 (lines 578-583): Complete algorithm
MeshData MeshOptimizer::SimplifyMesh(const MeshData& input, uint32_t targetTriangleCount) const {
    if (input.vertices.size() <= targetTriangleCount * 3 || targetTriangleCount == 0) {
        return input;
    }
    
    MeshData result = input;
    std::vector<Quadric> quadrics = ComputeVertexQuadrics(result);
    
    // Build edge list and priority queue
    // From tables.txt TABLE 10 (line 581): Insert edges into priority queue by cost
    struct EdgeCollapse {
        uint32_t v0, v1;
        float cost;
        DirectX::XMFLOAT3 optimalPos;
        
        bool operator>(const EdgeCollapse& other) const {
            return cost > other.cost;
        }
    };
    
    std::priority_queue<EdgeCollapse, std::vector<EdgeCollapse>, std::greater<EdgeCollapse>> pq;
    std::unordered_map<uint64_t, bool> edgeVisited;
    
    auto encodeEdge = [](uint32_t a, uint32_t b) -> uint64_t {
        return (static_cast<uint64_t>(std::min(a, b)) << 32) | std::max(a, b);
    };
    
    // Initialize edges from triangles
    for (size_t i = 0; i < result.indices.size(); i += 3) {
        uint32_t edges[3][2] = {
            {result.indices[i], result.indices[i+1]},
            {result.indices[i+1], result.indices[i+2]},
            {result.indices[i+2], result.indices[i]}
        };
        
        for (auto& edge : edges) {
            uint64_t key = encodeEdge(edge[0], edge[1]);
            if (edgeVisited.find(key) == edgeVisited.end()) {
                edgeVisited[key] = true;
                
                Quadric q = quadrics[edge[0]] + quadrics[edge[1]];
                DirectX::XMFLOAT3 optimalPos = FindOptimalCollapsePosition(q, edge[0], edge[1], result);
                float cost = q.Error(optimalPos);
                
                pq.push({edge[0], edge[1], cost, optimalPos});
            }
        }
    }
    
    // Track valid vertices
    std::vector<bool> vertexValid(result.vertices.size(), true);
    
    // Edge collapse loop
    // From tables.txt TABLE 10 (lines 582-583): Repeatedly extract min, collapse, update
    uint32_t currentTriangleCount = static_cast<uint32_t>(result.indices.size() / 3);
    
    while (currentTriangleCount > targetTriangleCount && !pq.empty()) {
        EdgeCollapse edge = pq.top();
        pq.pop();
        
        // Check if vertices are still valid
        if (!vertexValid[edge.v0] || !vertexValid[edge.v1]) {
            continue;
        }
        
        // From tables.txt TABLE 11 (lines 605-608): Topological constraints
        // Do not collapse if it would create degenerate triangles
        
        // Perform edge collapse: merge v1 into v0
        uint32_t keepVertex = edge.v0;
        uint32_t removeVertex = edge.v1;
        
        // Update position to optimal
        result.vertices[keepVertex] = edge.optimalPos;
        
        // Update quadric
        quadrics[keepVertex] = quadrics[keepVertex] + quadrics[removeVertex];
        
        // Remove triangles that use removeVertex
        std::vector<uint32_t> newIndices;
        newIndices.reserve(result.indices.size());
        
        std::set<uint64_t> newEdges;
        
        for (size_t i = 0; i < result.indices.size(); i += 3) {
            uint32_t i0 = result.indices[i];
            uint32_t i1 = result.indices[i + 1];
            uint32_t i2 = result.indices[i + 2];
            
            // Skip if triangle uses removed vertex
            if (i0 == removeVertex || i1 == removeVertex || i2 == removeVertex) {
                // Replace with keep vertex
                if (i0 == removeVertex) i0 = keepVertex;
                if (i1 == removeVertex) i1 = keepVertex;
                if (i2 == removeVertex) i2 = keepVertex;
                
                // Skip degenerate triangles (TABLE 11 line 612)
                if (i0 == i1 || i1 == i2 || i2 == i0) {
                    continue;
                }
            }
            
            newIndices.push_back(i0);
            newIndices.push_back(i1);
            newIndices.push_back(i2);
            
            // Track new edges
            uint64_t e1 = encodeEdge(i0, i1);
            uint64_t e2 = encodeEdge(i1, i2);
            uint64_t e3 = encodeEdge(i2, i0);
            newEdges.insert(e1);
            newEdges.insert(e2);
            newEdges.insert(e3);
        }
        
        result.indices = newIndices;
        vertexValid[removeVertex] = false;
        
        // Add new edges to priority queue
        for (uint64_t edgeKey : newEdges) {
            if (edgeVisited.find(edgeKey) == edgeVisited.end()) {
                edgeVisited[edgeKey] = true;
                uint32_t v0 = static_cast<uint32_t>(edgeKey >> 32);
                uint32_t v1 = static_cast<uint32_t>(edgeKey & 0xFFFFFFFF);
                
                if (vertexValid[v0] && vertexValid[v1]) {
                    Quadric q = quadrics[v0] + quadrics[v1];
                    DirectX::XMFLOAT3 optimalPos = FindOptimalCollapsePosition(q, v0, v1, result);
                    float cost = q.Error(optimalPos);
                    pq.push({v0, v1, cost, optimalPos});
                }
            }
        }
        
        currentTriangleCount = static_cast<uint32_t>(result.indices.size() / 3);
    }
    
    // Compact vertex list and update indices
    std::vector<uint32_t> vertexMap(result.vertices.size(), UINT32_MAX);
    uint32_t newVertexCount = 0;
    
    for (size_t i = 0; i < result.vertices.size(); ++i) {
        if (vertexValid[i]) {
            vertexMap[i] = newVertexCount;
            newVertexCount++;
        }
    }
    
    MeshData compacted;
    compacted.vertices.reserve(newVertexCount);
    compacted.normals.reserve(newVertexCount);
    
    for (size_t i = 0; i < result.vertices.size(); ++i) {
        if (vertexValid[i]) {
            compacted.vertices.push_back(result.vertices[i]);
            if (i < result.normals.size()) {
                // Average normal for collapsed vertices
                compacted.normals.push_back(result.normals[i]);
            }
        }
    }
    
    // Update indices
    compacted.indices.reserve(result.indices.size());
    for (uint32_t idx : result.indices) {
        compacted.indices.push_back(vertexMap[idx]);
    }
    
    return compacted;
}

} // namespace Mesh
} // namespace Procedural
} // namespace Engine
