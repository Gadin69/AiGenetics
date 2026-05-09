#include "Skeleton.h"
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

namespace Engine {
namespace Animation {

void Skeleton::AddBone(const Bone& bone, int32_t parentIndex)
{
    Bone newBone = bone;
    newBone.parentIndex = parentIndex;
    
    int32_t boneIndex = static_cast<int32_t>(m_bones.size());
    m_bones.push_back(newBone);
    
    // Expand children list
    while (m_children.size() <= boneIndex)
    {
        m_children.push_back(std::vector<int32_t>());
    }
    
    // Add to parent's children list
    if (parentIndex >= 0 && parentIndex < static_cast<int32_t>(m_children.size()))
    {
        m_children[parentIndex].push_back(boneIndex);
    }
}

void Skeleton::ComputeWorldTransforms()
{
    // Find root bones (parentIndex == -1)
    for (size_t i = 0; i < m_bones.size(); ++i)
    {
        if (m_bones[i].parentIndex == -1)
        {
            XMMATRIX identity = XMMatrixIdentity();
            XMFLOAT4X4 identityFloat;
            XMStoreFloat4x4(&identityFloat, identity);
            ComputeBoneTransformsRecursive(static_cast<int32_t>(i), identityFloat);
        }
    }
}

void Skeleton::ComputeBoneTransformsRecursive(int32_t boneIndex, const XMFLOAT4X4& parentWorldMatrix)
{
    if (boneIndex < 0 || boneIndex >= static_cast<int32_t>(m_bones.size()))
        return;
    
    Bone& bone = m_bones[boneIndex];
    
    // Build local transform from position and rotation
    XMVECTOR translation = XMLoadFloat3(&bone.localPosition);
    XMVECTOR rotation = XMLoadFloat3(&bone.localRotation);
    
    // Create rotation matrix from Euler angles
    XMMATRIX rotX = XMMatrixRotationX(rotation.m128_f32[0]);
    XMMATRIX rotY = XMMatrixRotationY(rotation.m128_f32[1]);
    XMMATRIX rotZ = XMMatrixRotationZ(rotation.m128_f32[2]);
    XMMATRIX rotationMatrix = rotZ * rotY * rotX;
    
    XMMATRIX translationMatrix = XMMatrixTranslationFromVector(translation);
    XMMATRIX localMatrix = rotationMatrix * translationMatrix;
    
    XMStoreFloat4x4(&bone.localTransform, localMatrix);
    
    // Compute world transform: parentWorld * localTransform
    XMMATRIX parentWorld = XMLoadFloat4x4(&parentWorldMatrix);
    XMMATRIX worldMatrix = parentWorld * localMatrix;
    XMStoreFloat4x4(&bone.worldTransform, worldMatrix);
    
    // Recursively compute children
    if (boneIndex >= 0 && boneIndex < static_cast<int32_t>(m_children.size()))
    {
        XMFLOAT4X4 worldFloat;
        XMStoreFloat4x4(&worldFloat, worldMatrix);
        
        for (int32_t childIndex : m_children[boneIndex])
        {
            ComputeBoneTransformsRecursive(childIndex, worldFloat);
        }
    }
}

void Skeleton::ApplyGeneticScaling(float scaleFactor)
{
    for (auto& bone : m_bones)
    {
        bone.localPosition.x *= scaleFactor;
        bone.localPosition.y *= scaleFactor;
        bone.localPosition.z *= scaleFactor;
        
        bone.boneLength.x *= scaleFactor;
        bone.boneLength.y *= scaleFactor;
        bone.boneLength.z *= scaleFactor;
    }
}

void Skeleton::ApplyLimbLengthModifiers(const std::vector<float>& modifiers)
{
    // Apply modifiers to limb bones (simplified - in practice, you'd identify limb bones by name or type)
    size_t modifierIndex = 0;
    for (auto& bone : m_bones)
    {
        if (modifierIndex < modifiers.size())
        {
            bone.boneLength.x *= modifiers[modifierIndex];
            bone.localPosition.z *= modifiers[modifierIndex]; // Assume Z is bone length axis
            modifierIndex++;
        }
    }
}

void Skeleton::AddStructuralConnection(int32_t boneIndex1, int32_t boneIndex2, 
                                       StructuralConnectionType type)
{
    if (boneIndex1 < 0 || boneIndex1 >= static_cast<int32_t>(m_bones.size())) return;
    if (boneIndex2 < 0 || boneIndex2 >= static_cast<int32_t>(m_bones.size())) return;
    
    m_bones[boneIndex1].structuralConnections.push_back(boneIndex2);
    m_bones[boneIndex1].connectionTypes.push_back(type);
    
    // Bidirectional
    m_bones[boneIndex2].structuralConnections.push_back(boneIndex1);
    m_bones[boneIndex2].connectionTypes.push_back(type);
}

const std::vector<int32_t>& Skeleton::GetStructuralConnections(int32_t boneIndex) const
{
    static std::vector<int32_t> empty;
    if (boneIndex < 0 || boneIndex >= static_cast<int32_t>(m_bones.size())) return empty;
    return m_bones[boneIndex].structuralConnections;
}

StructuralConnectionType Skeleton::GetConnectionType(int32_t boneIndex, size_t connectionIndex) const
{
    if (boneIndex < 0 || boneIndex >= static_cast<int32_t>(m_bones.size())) 
        return StructuralConnectionType::NONE;
    if (connectionIndex >= m_bones[boneIndex].connectionTypes.size()) 
        return StructuralConnectionType::NONE;
    return m_bones[boneIndex].connectionTypes[connectionIndex];
}

} // namespace Animation
} // namespace Engine
