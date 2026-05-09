#pragma once

#include "Bone.h"
#include <vector>
#include <DirectXMath.h>

namespace Engine {
namespace Animation {

// Skeleton class - manages hierarchical bone structure
class Skeleton {
private:
    std::vector<Bone> m_bones;
    std::vector<std::vector<int32_t>> m_children; // Adjacency list for hierarchy
    
public:
    Skeleton() = default;
    ~Skeleton() = default;
    
    // Add a bone to the skeleton
    void AddBone(const Bone& bone, int32_t parentIndex);
    
    // Compute world-space transforms for all bones (call each frame)
    void ComputeWorldTransforms();
    
    // Getters
    const std::vector<Bone>& GetBones() const { return m_bones; }
    const std::vector<std::vector<int32_t>>& GetHierarchy() const { return m_children; }
    size_t GetBoneCount() const { return m_bones.size(); }
    
    // Genetic influence methods
    void ApplyGeneticScaling(float scaleFactor);
    void ApplyLimbLengthModifiers(const std::vector<float>& modifiers);
    
private:
    // Recursive helper to compute world transforms
    void ComputeBoneTransformsRecursive(int32_t boneIndex, const DirectX::XMFLOAT4X4& parentWorldMatrix);
};

} // namespace Animation
} // namespace Engine
