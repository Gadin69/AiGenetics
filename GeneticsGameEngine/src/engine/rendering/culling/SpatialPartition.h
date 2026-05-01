#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "BoundingVolume.h"
#include "FrustumCuller.h"

namespace Engine {
namespace Rendering {

// Octree node for spatial partitioning
struct OctreeNode {
    DirectX::XMFLOAT3 center;
    float halfSize;
    std::vector<std::unique_ptr<OctreeNode>> children;
    std::vector<std::shared_ptr<BoundingVolume>> objects;
    
    OctreeNode(const DirectX::XMFLOAT3& c, float size) : center(c), halfSize(size) {}
    
    bool IsLeaf() const { return children.empty(); }
    
    // Check if point is inside this node
    bool Contains(const DirectX::XMFLOAT3& point) const {
        return (point.x >= center.x - halfSize && point.x <= center.x + halfSize) &&
               (point.y >= center.y - halfSize && point.y <= center.y + halfSize) &&
               (point.z >= center.z - halfSize && point.z <= center.z + halfSize);
    }
    
    // Get child index for a point
    int GetChildIndex(const DirectX::XMFLOAT3& point) const {
        int index = 0;
        if (point.x > center.x) index |= 1;
        if (point.y > center.y) index |= 2;
        if (point.z > center.z) index |= 4;
        return index;
    }
};

// Spatial partitioning system using octree structure
class SpatialPartition {
private:
    std::unique_ptr<OctreeNode> m_root;
    int m_maxDepth;
    int m_maxObjectsPerNode;
    
public:
    SpatialPartition(float worldSize = 1000.0f, int maxDepth = 8, int maxObjectsPerNode = 16)
        : m_maxDepth(maxDepth), m_maxObjectsPerNode(maxObjectsPerNode) {
        m_root = std::make_unique<OctreeNode>(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), worldSize / 2.0f);
    }
    
    // Insert object into spatial partition
    void Insert(const std::shared_ptr<BoundingVolume>& object, const DirectX::XMFLOAT3& center) {
        if (m_root) {
            InsertRecursive(m_root.get(), object, center, 0);
        }
    }
    
    // Query objects that might intersect with frustum
    std::vector<std::shared_ptr<BoundingVolume>> Query(const FrustumCuller& frustum) const {
        std::vector<std::shared_ptr<BoundingVolume>> results;
        if (m_root) {
            QueryRecursive(m_root.get(), frustum, results);
        }
        return results;
    }
    
    // Clear all objects
    void Clear() {
        if (m_root) {
            ClearRecursive(m_root.get());
        }
    }
    
private:
    void InsertRecursive(OctreeNode* node, const std::shared_ptr<BoundingVolume>& object, 
                        const DirectX::XMFLOAT3& center, int depth) {
        // If node is at max depth or has few objects, add to this node
        if (depth >= m_maxDepth || node->objects.size() < static_cast<size_t>(m_maxObjectsPerNode)) {
            node->objects.push_back(object);
            return;
        }
        
        // Otherwise, try to insert into child nodes
        int childIndex = node->GetChildIndex(center);
        
        // Create children if they don't exist
        if (node->children.empty()) {
            float childSize = node->halfSize / 2.0f;
            DirectX::XMFLOAT3 offsets[8] = {
                {-childSize, -childSize, -childSize}, // 0
                { childSize, -childSize, -childSize}, // 1
                {-childSize,  childSize, -childSize}, // 2
                { childSize,  childSize, -childSize}, // 3
                {-childSize, -childSize,  childSize}, // 4
                { childSize, -childSize,  childSize}, // 5
                {-childSize,  childSize,  childSize}, // 6
                { childSize,  childSize,  childSize}  // 7
            };
            
            for (int i = 0; i < 8; ++i) {
                DirectX::XMFLOAT3 childCenter;
                childCenter.x = node->center.x + offsets[i].x;
                childCenter.y = node->center.y + offsets[i].y;
                childCenter.z = node->center.z + offsets[i].z;
                node->children.push_back(std::make_unique<OctreeNode>(childCenter, childSize));
            }
        }
        
        // Insert into appropriate child
        if (childIndex < 8 && childIndex >= 0) {
            DirectX::XMFLOAT3 childCenter;
            childCenter.x = node->center.x + (childIndex & 1 ? node->halfSize/2.0f : -node->halfSize/2.0f);
            childCenter.y = node->center.y + (childIndex & 2 ? node->halfSize/2.0f : -node->halfSize/2.0f);
            childCenter.z = node->center.z + (childIndex & 4 ? node->halfSize/2.0f : -node->halfSize/2.0f);
            
            InsertRecursive(node->children[childIndex].get(), object, center, depth + 1);
        }
    }
    
    void QueryRecursive(const OctreeNode* node, const FrustumCuller& frustum, 
                       std::vector<std::shared_ptr<BoundingVolume>>& results) const {
        // Check if node intersects frustum
        if (!IntersectsFrustum(node, frustum)) {
            return;
        }
        
        // Add objects in this node
        for (const auto& object : node->objects) {
            results.push_back(object);
        }
        
        // Recursively check children
        for (const auto& child : node->children) {
            QueryRecursive(child.get(), frustum, results);
        }
    }
    
    void ClearRecursive(OctreeNode* node) {
        node->objects.clear();
        for (auto& child : node->children) {
            ClearRecursive(child.get());
        }
        node->children.clear();
    }
    
    bool IntersectsFrustum(const OctreeNode* node, const FrustumCuller& frustum) const {
        // Simple AABB-frustum intersection test
        DirectX::XMFLOAT3 minCorner(node->center.x - node->halfSize,
                                   node->center.y - node->halfSize,
                                   node->center.z - node->halfSize);
        DirectX::XMFLOAT3 maxCorner(node->center.x + node->halfSize,
                                   node->center.y + node->halfSize,
                                   node->center.z + node->halfSize);
        
        // Test against each frustum plane
        DirectX::XMFLOAT4 planes[6];
        // This is a simplified version - in practice we'd extract from view-projection matrix
        // For now, we'll use a basic AABB test
        return true; // Placeholder - will be implemented properly
    }
};

} // namespace Rendering
} // namespace Engine