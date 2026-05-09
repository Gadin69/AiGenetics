#include "ScalarFieldGenerator.h"
#include "../voxel/VoxelGrid.h"
#include <algorithm>
#include <cmath>

// Undefine windows.h max/min macros if defined
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace Engine {
namespace Procedural {
namespace Generation {

void ScalarFieldGenerator::GenerateField(Voxel::VoxelGrid& grid, const CreatureParams& params) {
    int sizeX = grid.GetSizeX();
    int sizeY = grid.GetSizeY();
    int sizeZ = grid.GetSizeZ();
    float voxelSize = grid.GetVoxelSize();
    
    // Calculate offset to center the creature in the grid
    float offsetX = -sizeX * voxelSize * 0.5f;
    float offsetY = -sizeY * voxelSize * 0.5f;
    float offsetZ = -sizeZ * voxelSize * 0.5f;
    
    // Fill scalar field by evaluating density function at each voxel position
    for (int z = 0; z < sizeZ; ++z) {
        for (int y = 0; y < sizeY; ++y) {
            for (int x = 0; x < sizeX; ++x) {
                // Convert voxel coordinates to world space
                DirectX::XMFLOAT3 pos(
                    x * voxelSize + offsetX,
                    y * voxelSize + offsetY,
                    z * voxelSize + offsetZ
                );
                
                // Evaluate density function
                float density = CreatureDensityFunction(pos, params);
                
                // Store in scalar field
                grid.SetScalarField(x, y, z, density);
            }
        }
    }
}

// Main creature density function
// From tables.txt TABLE 8 (lines 491-512)
float ScalarFieldGenerator::CreatureDensityFunction(const DirectX::XMFLOAT3& pos, const CreatureParams& params) const {
    float density = 0.0f;
    
    // Body core (ellipsoid)
    float bodyDensity = EllipsoidDensity(pos, params.bodyCenter, params.bodyRadii);
    density += std::max(0.0f, bodyDensity);
    
    // Limbs (cylinders)
    float limbRadius = 0.1f * params.scaleFactor;
    float bodyLength = params.bodyRadii.x;
    
    for (int i = 0; i < params.limbCount; ++i) {
        // Calculate limb start and end positions
        float angle = (2.0f * 3.14159265f * i) / params.limbCount;
        float limbLength = 0.8f * params.scaleFactor;
        
        DirectX::XMFLOAT3 limbStart(
            params.bodyCenter.x + cos(angle) * params.bodyRadii.y * 0.8f,
            params.bodyCenter.y - 0.2f * params.scaleFactor,
            params.bodyCenter.z + sin(angle) * params.bodyRadii.z * 0.8f
        );
        
        DirectX::XMFLOAT3 limbEnd(
            limbStart.x + cos(angle) * limbLength,
            limbStart.y - limbLength * 0.5f,
            limbStart.z + sin(angle) * limbLength
        );
        
        float limbDist = DistanceToCylinder(pos, limbStart, limbEnd, limbRadius);
        density += Smoothstep(limbRadius + 0.1f, limbRadius - 0.1f, limbDist);
    }
    
    // Head (sphere)
    DirectX::XMVECTOR headPos = DirectX::XMLoadFloat3(&pos);
    DirectX::XMVECTOR headCenterVec = DirectX::XMLoadFloat3(&params.headCenter);
    float headDist = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(headPos, headCenterVec)));
    density += Smoothstep(params.headRadius * 1.2f, params.headRadius * 0.8f, headDist);
    
    // Apply internal cavity if specified
    if (params.internalCavity > 0.0f) {
        float cavityDensity = EllipsoidDensity(pos, params.bodyCenter, 
            DirectX::XMFLOAT3(
                params.bodyRadii.x * (1.0f - params.internalCavity),
                params.bodyRadii.y * (1.0f - params.internalCavity),
                params.bodyRadii.z * (1.0f - params.internalCavity)
            ));
        density -= std::max(0.0f, cavityDensity) * params.internalCavity;
    }
    
    // Return density - isolevel (0.5 from tables.txt TABLE 8 line 511)
    return density - 0.5f;
}

// Calculate shortest distance from point to cylinder
float ScalarFieldGenerator::DistanceToCylinder(const DirectX::XMFLOAT3& pos, 
                                               const DirectX::XMFLOAT3& start, 
                                               const DirectX::XMFLOAT3& end, 
                                               float radius) const {
    // Cylinder axis vector
    DirectX::XMVECTOR axis = DirectX::XMVectorSubtract(
        DirectX::XMLoadFloat3(&end),
        DirectX::XMLoadFloat3(&start)
    );
    
    // Vector from start to point
    DirectX::XMVECTOR toPoint = DirectX::XMVectorSubtract(
        DirectX::XMLoadFloat3(&pos),
        DirectX::XMLoadFloat3(&start)
    );
    
    // Project toPoint onto axis
    float axisLengthSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(axis));
    if (axisLengthSq < 0.0001f) {
        // Degenerate cylinder (start == end), treat as sphere
        return DirectX::XMVectorGetX(DirectX::XMVector3Length(toPoint)) - radius;
    }
    
    float t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(toPoint, axis)) / axisLengthSq;
    t = std::clamp(t, 0.0f, 1.0f);
    
    // Closest point on cylinder axis
    DirectX::XMVECTOR closestPoint = DirectX::XMVectorAdd(
        DirectX::XMLoadFloat3(&start),
        DirectX::XMVectorScale(axis, t)
    );
    
    // Distance from point to closest point on axis
    float distToAxis = DirectX::XMVectorGetX(
        DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&pos), closestPoint))
    );
    
    return distToAxis - radius;
}

// Smoothstep function for smooth transitions
// From tables.txt TABLE 8 line 515
float ScalarFieldGenerator::Smoothstep(float edge0, float edge1, float x) const {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Ellipsoid density function
// Returns 1.0 at center, 0.0 at surface, negative outside
float ScalarFieldGenerator::EllipsoidDensity(const DirectX::XMFLOAT3& pos, 
                                            const DirectX::XMFLOAT3& center, 
                                            const DirectX::XMFLOAT3& radii) const {
    DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&pos);
    DirectX::XMVECTOR c = DirectX::XMLoadFloat3(&center);
    DirectX::XMVECTOR r = DirectX::XMLoadFloat3(&radii);
    
    // Normalized distance from center
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(p, c);
    DirectX::XMVECTOR normalized = DirectX::XMVectorDivide(diff, r);
    
    float distSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(normalized));
    
    return 1.0f - std::sqrt(distSq);
}

// Sphere density function
// Returns 1.0 at center, 0.0 at surface, negative outside
float ScalarFieldGenerator::SphereDensity(const DirectX::XMFLOAT3& pos, 
                                         const DirectX::XMFLOAT3& center, 
                                         float radius) const {
    DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&pos);
    DirectX::XMVECTOR c = DirectX::XMLoadFloat3(&center);
    float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(p, c)));
    return 1.0f - (dist / radius);
}

// NEW: Generate scalar field from skeleton (bones act as metaball attractors)
void ScalarFieldGenerator::GenerateFieldFromSkeleton(
    Voxel::VoxelGrid& grid,
    const Engine::Animation::Skeleton& skeleton,
    const CreatureParams& params)
{
    int sizeX = grid.GetSizeX();
    int sizeY = grid.GetSizeY();
    int sizeZ = grid.GetSizeZ();
    float voxelSize = grid.GetVoxelSize();
    
    // Calculate offset to center the creature in the grid
    float offsetX = -sizeX * voxelSize * 0.5f;
    float offsetY = -sizeY * voxelSize * 0.5f;
    float offsetZ = -sizeZ * voxelSize * 0.5f;
    
    const auto& bones = skeleton.GetBones();
    
    // Fill scalar field by computing distance to nearest bone at each voxel
    for (int z = 0; z < sizeZ; ++z) {
        for (int y = 0; y < sizeY; ++y) {
            for (int x = 0; x < sizeX; ++x) {
                // Convert voxel coordinates to world space
                DirectX::XMFLOAT3 voxelPos(
                    x * voxelSize + offsetX,
                    y * voxelSize + offsetY,
                    z * voxelSize + offsetZ
                );
                
                // Compute density from all bones (metaball-style summation)
                float density = 0.0f;
                for (const auto& bone : bones)
                {
                    float boneDensity = ComputeBoneDensity(voxelPos, bone);
                    density += boneDensity;
                }
                
                // Store in scalar field
                grid.SetScalarField(x, y, z, density);
            }
        }
    }
}

// Compute density contribution from a single bone
float ScalarFieldGenerator::ComputeBoneDensity(
    const DirectX::XMFLOAT3& voxelPos,
    const Engine::Animation::Bone& bone) const
{
    // Get bone's world-space position from transform matrix
    DirectX::XMFLOAT3 bonePos = {
        bone.worldTransform._41,
        bone.worldTransform._42,
        bone.worldTransform._43
    };
    
    // Bone thickness (average of width and height)
    float boneRadius = (bone.boneLength.y + bone.boneLength.z) * 0.5f;
    
    // Bone length direction (Z axis in local space, transformed to world)
    DirectX::XMFLOAT3 boneDir = {
        bone.worldTransform._31,
        bone.worldTransform._32,
        bone.worldTransform._33
    };
    
    // Compute bone endpoint
    DirectX::XMFLOAT3 boneEnd = {
        bonePos.x + boneDir.x * bone.boneLength.x,
        bonePos.y + boneDir.y * bone.boneLength.x,
        bonePos.z + boneDir.z * bone.boneLength.x
    };
    
    // Use cylinder SDF for bone density
    return CylinderSDF(voxelPos, bonePos, boneEnd, boneRadius);
}

// Cylinder signed distance function
float ScalarFieldGenerator::CylinderSDF(
    const DirectX::XMFLOAT3& pos,
    const DirectX::XMFLOAT3& boneStart,
    const DirectX::XMFLOAT3& boneEnd,
    float radius) const
{
    // Cylinder axis
    DirectX::XMFLOAT3 axis = {
        boneEnd.x - boneStart.x,
        boneEnd.y - boneStart.y,
        boneEnd.z - boneStart.z
    };
    
    float axisLength = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    if (axisLength < 0.001f) return 0.0f; // Degenerate bone
    
    // Normalize axis
    axis.x /= axisLength;
    axis.y /= axisLength;
    axis.z /= axisLength;
    
    // Vector from bone start to point
    DirectX::XMFLOAT3 toPoint = {
        pos.x - boneStart.x,
        pos.y - boneStart.y,
        pos.z - boneStart.z
    };
    
    // Project point onto axis
    float projection = toPoint.x * axis.x + toPoint.y * axis.y + toPoint.z * axis.z;
    
    // Clamp projection to bone length
    projection = std::max(0.0f, std::min(projection, axisLength));
    
    // Closest point on cylinder axis
    DirectX::XMFLOAT3 closestPoint = {
        boneStart.x + axis.x * projection,
        boneStart.y + axis.y * projection,
        boneStart.z + axis.z * projection
    };
    
    // Distance from point to closest point on axis
    DirectX::XMFLOAT3 distVec = {
        pos.x - closestPoint.x,
        pos.y - closestPoint.y,
        pos.z - closestPoint.z
    };
    
    float distance = std::sqrt(distVec.x * distVec.x + distVec.y * distVec.y + distVec.z * distVec.z);
    
    // Density falls off with distance (metaball-style)
    float falloff = radius * 2.0f; // Influence radius
    return std::max(0.0f, 1.0f - (distance / falloff));
}

} // namespace Generation
} // namespace Procedural
} // namespace Engine
