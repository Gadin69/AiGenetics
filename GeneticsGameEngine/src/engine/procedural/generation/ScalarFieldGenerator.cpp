#include "ScalarFieldGenerator.h"
#include "../voxel/VoxelGrid.h"
#include "../../animation/Skeleton.h"
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
// Helper: Calculate joint position between parent and child bones
static DirectX::XMFLOAT3 CalculateJointPosition(
    const Engine::Animation::Bone& parent,
    const Engine::Animation::Bone& child)
{
    // Joint is at parent's endpoint (which should equal child's start)
    // Use average to handle any small misalignment
    DirectX::XMFLOAT3 jointPos = {
        (parent.worldEndpoint.x + child.worldTransform._41) * 0.5f,
        (parent.worldEndpoint.y + child.worldTransform._42) * 0.5f,
        (parent.worldEndpoint.z + child.worldTransform._43) * 0.5f
    };
    return jointPos;
}

// Helper: Calculate appropriate joint radius
static float CalculateJointRadius(
    const Engine::Animation::Bone& parent,
    const Engine::Animation::Bone& child)
{
    // Average the bone radii, then scale up slightly to ensure overlap
    float parentRadius = (parent.boneLength.x + parent.boneLength.y + parent.boneLength.z) / 3.0f;
    float childRadius = (child.boneLength.x + child.boneLength.y + child.boneLength.z) / 3.0f;
    float avgRadius = (parentRadius + childRadius) * 0.5f;
    
    // Scale up by 1.2x (reduced from 1.5x) - just enough to guarantee connectivity
    return avgRadius * 1.2f;
}

// Helper: Test if a voxel position is inside a bone (binary test, no SDF)
static bool IsVoxelInsideBone(
    const DirectX::XMFLOAT3& voxelPos,
    const Engine::Animation::Bone& bone,
    Engine::Procedural::Generation::ArchetypeType archetype)
{
    // Get bone's world-space position
    DirectX::XMFLOAT3 bonePos = {
        bone.worldTransform._41,
        bone.worldTransform._42,
        bone.worldTransform._43
    };
    
    // Bone radius (average dimension)
    float boneRadius = (bone.boneLength.x + bone.boneLength.y + bone.boneLength.z) / 3.0f;
    
    // Compute bone endpoint
    DirectX::XMFLOAT3 boneEnd = bone.worldEndpoint;
    
    // If worldEndpoint not computed (fallback), calculate from boneLength
    if (boneEnd.x == 0.0f && boneEnd.y == 0.0f && boneEnd.z == 0.0f && bone.parentIndex != -1) {
        float maxX = std::abs(bone.boneLength.x);
        float maxY = std::abs(bone.boneLength.y);
        float maxZ = std::abs(bone.boneLength.z);
        
        if (maxY >= maxX && maxY >= maxZ)
            boneEnd = { bonePos.x, bonePos.y + bone.boneLength.y, bonePos.z };
        else if (maxX >= maxY && maxX >= maxZ)
            boneEnd = { bonePos.x + bone.boneLength.x, bonePos.y, bonePos.z };
        else
            boneEnd = { bonePos.x, bonePos.y, bonePos.z + bone.boneLength.z };
    }
    
    // Test based on archetype
    switch (archetype)
    {
        case Engine::Procedural::Generation::ArchetypeType::Chordata:
        {
            // Capsule: distance to line segment <= radius
            DirectX::XMFLOAT3 ab = { boneEnd.x - bonePos.x, boneEnd.y - bonePos.y, boneEnd.z - bonePos.z };
            DirectX::XMFLOAT3 ap = { voxelPos.x - bonePos.x, voxelPos.y - bonePos.y, voxelPos.z - bonePos.z };
            
            float abLenSq = ab.x * ab.x + ab.y * ab.y + ab.z * ab.z;
            if (abLenSq < 0.0001f) {
                // Degenerate capsule (point), test sphere
                float distSq = ap.x * ap.x + ap.y * ap.y + ap.z * ap.z;
                return distSq <= boneRadius * boneRadius;
            }
            
            float t = (ap.x * ab.x + ap.y * ab.y + ap.z * ab.z) / abLenSq;
            t = std::clamp(t, 0.0f, 1.0f);
            
            DirectX::XMFLOAT3 closest = {
                bonePos.x + t * ab.x,
                bonePos.y + t * ab.y,
                bonePos.z + t * ab.z
            };
            
            DirectX::XMFLOAT3 diff = {
                voxelPos.x - closest.x,
                voxelPos.y - closest.y,
                voxelPos.z - closest.z
            };
            float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            return distSq <= boneRadius * boneRadius;
        }
        
        case Engine::Procedural::Generation::ArchetypeType::Arthropoda:
        {
            // Cylinder: same as capsule but with flat ends
            DirectX::XMFLOAT3 ab = { boneEnd.x - bonePos.x, boneEnd.y - bonePos.y, boneEnd.z - bonePos.z };
            DirectX::XMFLOAT3 ap = { voxelPos.x - bonePos.x, voxelPos.y - bonePos.y, voxelPos.z - bonePos.z };
            
            float abLenSq = ab.x * ab.x + ab.y * ab.y + ab.z * ab.z;
            if (abLenSq < 0.0001f) {
                float distSq = ap.x * ap.x + ap.y * ap.y + ap.z * ap.z;
                return distSq <= boneRadius * boneRadius;
            }
            
            float t = (ap.x * ab.x + ap.y * ab.y + ap.z * ab.z) / abLenSq;
            
            // For cylinder, t must be in [0, 1] (no rounding at ends)
            if (t < 0.0f || t > 1.0f) return false;
            
            DirectX::XMFLOAT3 closest = {
                bonePos.x + t * ab.x,
                bonePos.y + t * ab.y,
                bonePos.z + t * ab.z
            };
            
            DirectX::XMFLOAT3 diff = {
                voxelPos.x - closest.x,
                voxelPos.y - closest.y,
                voxelPos.z - closest.z
            };
            float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            return distSq <= boneRadius * boneRadius;
        }
        
        case Engine::Procedural::Generation::ArchetypeType::Mollusca:
        {
            // Sphere at midpoint (metaball approximation)
            DirectX::XMFLOAT3 center = {
                (bonePos.x + boneEnd.x) * 0.5f,
                (bonePos.y + boneEnd.y) * 0.5f,
                (bonePos.z + boneEnd.z) * 0.5f
            };
            
            DirectX::XMFLOAT3 diff = {
                voxelPos.x - center.x,
                voxelPos.y - center.y,
                voxelPos.z - center.z
            };
            float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            return distSq <= boneRadius * boneRadius; // No 1.3x multiplier for binary test
        }
        
        default:
        {
            // Fallback to capsule test
            DirectX::XMFLOAT3 ab = { boneEnd.x - bonePos.x, boneEnd.y - bonePos.y, boneEnd.z - bonePos.z };
            DirectX::XMFLOAT3 ap = { voxelPos.x - bonePos.x, voxelPos.y - bonePos.y, voxelPos.z - bonePos.z };
            
            float abLenSq = ab.x * ab.x + ab.y * ab.y + ab.z * ab.z;
            if (abLenSq < 0.0001f) {
                float distSq = ap.x * ap.x + ap.y * ap.y + ap.z * ap.z;
                return distSq <= boneRadius * boneRadius;
            }
            
            float t = (ap.x * ab.x + ap.y * ab.y + ap.z * ab.z) / abLenSq;
            t = std::clamp(t, 0.0f, 1.0f);
            
            DirectX::XMFLOAT3 closest = {
                bonePos.x + t * ab.x,
                bonePos.y + t * ab.y,
                bonePos.z + t * ab.z
            };
            
            DirectX::XMFLOAT3 diff = {
                voxelPos.x - closest.x,
                voxelPos.y - closest.y,
                voxelPos.z - closest.z
            };
            float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            return distSq <= boneRadius * boneRadius;
        }
    }
}

void ScalarFieldGenerator::GenerateFieldFromSkeleton(
    Voxel::VoxelGrid& grid,
    const Engine::Animation::Skeleton& skeleton,
    const CreatureParams& params)
{
    int sizeX = grid.GetSizeX();
    int sizeY = grid.GetSizeY();
    int sizeZ = grid.GetSizeZ();
    float voxelSize = grid.GetVoxelSize();
    
    const auto& bones = skeleton.GetBones();
    
    // Calculate bounding box from ALL bone world positions and their lengths
    // This ensures the grid encompasses the entire creature
    float minX = 999.0f, minY = 999.0f, minZ = 999.0f;
    float maxX = -999.0f, maxY = -999.0f, maxZ = -999.0f;
    
    for (const auto& bone : bones)
    {
        // Get bone world position
        DirectX::XMFLOAT3 worldPos = {
            bone.worldTransform._41,
            bone.worldTransform._42,
            bone.worldTransform._43
        };
        
        // Include bone position
        minX = std::min(minX, worldPos.x);
        minY = std::min(minY, worldPos.y);
        minZ = std::min(minZ, worldPos.z);
        maxX = std::max(maxX, worldPos.x);
        maxY = std::max(maxY, worldPos.y);
        maxZ = std::max(maxZ, worldPos.z);
        
        // Include bone extent (use max dimension as radius)
        float boneRadius = std::max({std::abs(bone.boneLength.x), std::abs(bone.boneLength.y), std::abs(bone.boneLength.z)});
        minX = std::min(minX, worldPos.x - boneRadius);
        minY = std::min(minY, worldPos.y - boneRadius);
        minZ = std::min(minZ, worldPos.z - boneRadius);
        maxX = std::max(maxX, worldPos.x + boneRadius);
        maxY = std::max(maxY, worldPos.y + boneRadius);
        maxZ = std::max(maxZ, worldPos.z + boneRadius);
    }
    
    // Add padding to ensure smooth falloff at edges and allow mesh to close off
    float padding = voxelSize * 10.0f; // 10 voxels of padding for proper mesh closure
    minX -= padding;
    minY -= padding;
    minZ -= padding;
    maxX += padding;
    maxY += padding;
    maxZ += padding;
    
    // NEW: Collect joint positions and radii for all parent-child connections
    struct JointInfo {
        DirectX::XMFLOAT3 position;
        float radius;
    };
    std::vector<JointInfo> joints;
    
    for (size_t i = 0; i < bones.size(); ++i)
    {
        if (bones[i].parentIndex >= 0 && bones[i].parentIndex < static_cast<int32_t>(bones.size()))
        {
            const Engine::Animation::Bone& parent = bones[bones[i].parentIndex];
            const Engine::Animation::Bone& child = bones[i];
            
            JointInfo joint;
            joint.position = CalculateJointPosition(parent, child);
            joint.radius = CalculateJointRadius(parent, child);
            
            joints.push_back(joint);
            
            // Expand bounds to include joint spheres
            minX = std::min(minX, joint.position.x - joint.radius);
            minY = std::min(minY, joint.position.y - joint.radius);
            minZ = std::min(minZ, joint.position.z - joint.radius);
            maxX = std::max(maxX, joint.position.x + joint.radius);
            maxY = std::max(maxY, joint.position.y + joint.radius);
            maxZ = std::max(maxZ, joint.position.z + joint.radius);
        }
    }
    
    printf("  [DEBUG ScalarField] Added %zu joint spheres for connectivity\n", joints.size());
    
    // Calculate grid center and offset
    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;
    
    // Offset places the grid so it encompasses the creature
    float offsetX = centerX - sizeX * voxelSize * 0.5f;
    float offsetY = centerY - sizeY * voxelSize * 0.5f;
    float offsetZ = centerZ - sizeZ * voxelSize * 0.5f;
    
    printf("  [DEBUG ScalarField] Bounds: (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f), Center: (%.2f, %.2f, %.2f)\n",
           minX, minY, minZ, maxX, maxY, maxZ, centerX, centerY, centerZ);
    
    // DEBUG: Print ALL bone world positions and parent indices to see limb bones
    static bool showedAllBones = false;
    bool showAllBones = !showedAllBones; // Show all bones for first creature only
    if (showAllBones) showedAllBones = true;
    
    printf("  [DEBUG BoneTransforms] Total bones: %zu%s\n", bones.size(), showAllBones ? " (showing all)" : "");
    
    size_t maxBonesToShow = showAllBones ? bones.size() : 5;
    for (size_t i = 0; i < bones.size() && i < maxBonesToShow; i++)
    {
        DirectX::XMFLOAT3 worldPos = {
            bones[i].worldTransform._41,
            bones[i].worldTransform._42,
            bones[i].worldTransform._43
        };
        printf("  [DEBUG BoneTransforms] Bone %zu: %s, parent=%d, localPos=(%.2f, %.2f, %.2f), boneLength=(%.2f, %.2f, %.2f), worldPos=(%.2f, %.2f, %.2f)\n",
               i, bones[i].name.c_str(), bones[i].parentIndex,
               bones[i].localPosition.x, bones[i].localPosition.y, bones[i].localPosition.z,
               bones[i].boneLength.x, bones[i].boneLength.y, bones[i].boneLength.z,
               worldPos.x, worldPos.y, worldPos.z);
    }
    
    // Calculate bone statistics for debugging only (no adaptive scaling)
    float minBoneRadius = 999.0f;
    float maxBoneRadius = 0.0f;
    float avgBoneRadius = 0.0f;
    for (const auto& bone : bones)
    {
        float radius = (bone.boneLength.x + bone.boneLength.y + bone.boneLength.z) / 3.0f;
        minBoneRadius = std::min(minBoneRadius, radius);
        maxBoneRadius = std::max(maxBoneRadius, radius);
        avgBoneRadius += radius;
    }
    avgBoneRadius /= bones.size();
    
    // NO ADAPTIVE SCALING - use actual bone dimensions directly
    // The boneLength values from the skeleton generator already define the correct size
    float adaptiveScale = 1.0f; // No scaling - bones are already the right size
    
    printf("  [DEBUG ScalarField] Bone radius range: [%.3f, %.3f], Avg: %.3f, Scale: %.3f (no scaling)\n", 
           minBoneRadius, maxBoneRadius, avgBoneRadius, adaptiveScale);
    
    // ADAPTIVE VOXEL DENSITY: Compute resolution multiplier field
    // - Fine resolution (0.5x) near joints and bone endpoints (high curvature areas)
    // - Base resolution (1.0x) near bone surfaces
    // - Coarse resolution (2.0x) in empty space far from creature
    bool useAdaptiveDensity = true;
    int refinedVoxels = 0;
    int coarsenedVoxels = 0;
    
    if (useAdaptiveDensity)
    {
        grid.EnableAdaptiveResolution(true);
        
        for (int z = 0; z < sizeZ; ++z) {
            for (int y = 0; y < sizeY; ++y) {
                for (int x = 0; x < sizeX; ++x) {
                    DirectX::XMFLOAT3 voxelPos(
                        x * voxelSize + offsetX,
                        y * voxelSize + offsetY,
                        z * voxelSize + offsetZ
                    );
                    
                    // Compute minimum distance to any joint
                    float minJointDist = 999.0f;
                    for (const auto& joint : joints)
                    {
                        DirectX::XMFLOAT3 diff = {
                            voxelPos.x - joint.position.x,
                            voxelPos.y - joint.position.y,
                            voxelPos.z - joint.position.z
                        };
                        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
                        minJointDist = std::min(minJointDist, dist);
                    }
                    
                    // Compute minimum distance to any bone surface
                    float minBoneSurfaceDist = 999.0f;
                    for (const auto& bone : bones)
                    {
                        DirectX::XMFLOAT3 bonePos = {
                            bone.worldTransform._41,
                            bone.worldTransform._42,
                            bone.worldTransform._43
                        };
                        float boneRadius = (bone.boneLength.x + bone.boneLength.y + bone.boneLength.z) / 3.0f;
                        
                        DirectX::XMFLOAT3 diff = {
                            voxelPos.x - bonePos.x,
                            voxelPos.y - bonePos.y,
                            voxelPos.z - bonePos.z
                        };
                        float distToCenter = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
                        float distToSurface = std::abs(distToCenter - boneRadius);
                        minBoneSurfaceDist = std::min(minBoneSurfaceDist, distToSurface);
                    }
                    
                    // Adaptive resolution logic:
                    // - Very close to joints (< 0.3 units): 0.5x resolution (2x finer)
                    // - Close to joints (< 0.6 units): 0.7x resolution
                    // - Near bone surfaces (< 0.2 units): 0.8x resolution
                    // - Far from everything (> 1.0 units): 1.5x resolution (coarser)
                    float resolutionMultiplier = 1.0f;
                    
                    if (minJointDist < 0.3f)
                    {
                        resolutionMultiplier = 0.5f; // 2x finer near joints
                        refinedVoxels++;
                    }
                    else if (minJointDist < 0.6f)
                    {
                        resolutionMultiplier = 0.7f;
                        refinedVoxels++;
                    }
                    else if (minBoneSurfaceDist < 0.2f)
                    {
                        resolutionMultiplier = 0.8f;
                        refinedVoxels++;
                    }
                    else if (minBoneSurfaceDist > 1.0f && minJointDist > 1.0f)
                    {
                        resolutionMultiplier = 1.5f; // 1.5x coarser in empty space
                        coarsenedVoxels++;
                    }
                    
                    grid.SetResolutionMultiplier(x, y, z, resolutionMultiplier);
                }
            }
        }
        
        printf("  [DEBUG AdaptiveResolution] Refined: %d voxels, Coarsened: %d voxels\n", 
               refinedVoxels, coarsenedVoxels);
    }
    
    // DIRECT BINARY VOXELIZATION: Test if each voxel is inside any bone or joint
    // No blending, no normalization - just solid where bones are, empty where they're not
    // ADAPTIVE: Use resolution multiplier to determine sampling density
    int voxelsSolid = 0;
    int voxelsEmpty = 0;
    int subVoxelSamples = 0;
    
    for (int z = 0; z < sizeZ; ++z) {
        for (int y = 0; y < sizeY; ++y) {
            for (int x = 0; x < sizeX; ++x) {
                // Convert voxel coordinates to world space
                DirectX::XMFLOAT3 voxelPos(
                    x * voxelSize + offsetX,
                    y * voxelSize + offsetY,
                    z * voxelSize + offsetZ
                );
                
                // Get resolution multiplier for this voxel
                float resolutionMult = 1.0f;
                if (grid.IsAdaptiveResolutionEnabled())
                {
                    resolutionMult = grid.GetResolutionMultiplier(x, y, z);
                }
                
                // Determine number of sub-voxel samples based on resolution
                // - 0.5x resolution = 2x2x2 = 8 samples
                // - 0.7x resolution = 2x2x2 = 8 samples
                // - 0.8x resolution = 1x1x1 = 1 sample
                // - 1.0x resolution = 1x1x1 = 1 sample
                // - 1.5x resolution = 1x1x1 = 1 sample (skip detail)
                int samplesPerAxis = 1;
                if (resolutionMult <= 0.5f)
                    samplesPerAxis = 3; // 27 samples for high detail
                else if (resolutionMult <= 0.7f)
                    samplesPerAxis = 2; // 8 samples for medium detail
                else if (resolutionMult <= 0.8f)
                    samplesPerAxis = 2; // 8 samples for surface detail
                
                float subVoxelSize = voxelSize / samplesPerAxis;
                
                // Test if voxel is inside ANY bone using sub-voxel sampling
                bool isInside = false;
                int samplesInside = 0;
                int totalSamples = 0;
                
                for (int sz = 0; sz < samplesPerAxis; ++sz) {
                    for (int sy = 0; sy < samplesPerAxis; ++sy) {
                        for (int sx = 0; sx < samplesPerAxis; ++sx) {
                            DirectX::XMFLOAT3 samplePos(
                                voxelPos.x + (sx - (samplesPerAxis - 1) * 0.5f) * subVoxelSize,
                                voxelPos.y + (sy - (samplesPerAxis - 1) * 0.5f) * subVoxelSize,
                                voxelPos.z + (sz - (samplesPerAxis - 1) * 0.5f) * subVoxelSize
                            );
                            
                            totalSamples++;
                            subVoxelSamples++;
                            
                            // Check all bones
                            bool sampleInside = false;
                            for (const auto& bone : bones)
                            {
                                if (IsVoxelInsideBone(samplePos, bone, params.archetype))
                                {
                                    sampleInside = true;
                                    break;
                                }
                            }
                            
                            // Check all joint spheres if not already inside a bone
                            if (!sampleInside)
                            {
                                for (const auto& joint : joints)
                                {
                                    DirectX::XMFLOAT3 toCenter = {
                                        samplePos.x - joint.position.x,
                                        samplePos.y - joint.position.y,
                                        samplePos.z - joint.position.z
                                    };
                                    float dist = std::sqrt(
                                        toCenter.x * toCenter.x + 
                                        toCenter.y * toCenter.y + 
                                        toCenter.z * toCenter.z
                                    );
                                    if (dist <= joint.radius)
                                    {
                                        sampleInside = true;
                                        break;
                                    }
                                }
                            }
                            
                            if (sampleInside)
                                samplesInside++;
                        }
                    }
                }
                
                // Determine final density based on sample ratio
                // For adaptive resolution: require higher sample ratio for coarse voxels
                // to avoid false positives in empty space
                float sampleRatio = static_cast<float>(samplesInside) / totalSamples;
                float threshold = (resolutionMult > 1.0f) ? 0.5f : 0.3f;
                
                float density = (sampleRatio >= threshold) ? 1.0f : 0.0f;
                
                if (density > 0.5f) voxelsSolid++;
                else voxelsEmpty++;
                
                grid.SetScalarField(x, y, z, density);
            }
        }
    }
    
    // Debug output
    printf("  [DEBUG ScalarField] Voxels: %d solid (bones/joints), %d empty (total: %d)\n", 
           voxelsSolid, voxelsEmpty, sizeX * sizeY * sizeZ);
    printf("  [DEBUG ScalarField] Sub-voxel samples: %d (avg %.1f per voxel)\n",
           subVoxelSamples, static_cast<float>(subVoxelSamples) / (sizeX * sizeY * sizeZ));
    
    // PASS 2: Smoothing pass - blend edges for organic look
    // Create a copy of the scalar field to read from while we write smoothed values
    const float* originalData = grid.GetScalarFieldPointer();
    std::vector<float> originalField(originalData, originalData + (sizeX * sizeY * sizeZ));
    int smoothedCount = 0;
    
    for (int z = 1; z < sizeZ - 1; ++z) {
        for (int y = 1; y < sizeY - 1; ++y) {
            for (int x = 1; x < sizeX - 1; ++x) {
                // Only smooth boundary voxels (where density is 0 or 1)
                float currentDensity = originalField[(z * sizeY + y) * sizeX + x];
                
                // Skip if already in the middle range (already smoothed)
                if (currentDensity > 0.05f && currentDensity < 0.95f)
                    continue;
                
                // Average with 26 neighbors (3x3x3 kernel excluding center)
                float sum = 0.0f;
                int count = 0;
                
                for (int dz = -1; dz <= 1; ++dz) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {
                            if (dx == 0 && dy == 0 && dz == 0)
                                continue; // Skip center
                            
                            sum += originalField[((z + dz) * sizeY + (y + dy)) * sizeX + (x + dx)];
                            count++;
                        }
                    }
                }
                
                float smoothedDensity = sum / count;
                grid.SetScalarField(x, y, z, smoothedDensity);
                smoothedCount++;
            }
        }
    }
    
    printf("  [DEBUG ScalarField] Smoothed %d boundary voxels for organic blending\n", smoothedCount);
}

// Compute SDF contribution from a single bone (returns true signed distance)
float ScalarFieldGenerator::ComputeBoneSDF(
    const DirectX::XMFLOAT3& voxelPos,
    const Engine::Animation::Bone& bone,
    float adaptiveScale,
    ArchetypeType archetype) const
{
    // Get bone's world-space position from transform matrix
    DirectX::XMFLOAT3 bonePos = {
        bone.worldTransform._41,
        bone.worldTransform._42,
        bone.worldTransform._43
    };
    
    // Bone thickness (use average dimension)
    // Use 3.0f divisor to match the radius calculation in GenerateFieldFromSkeleton
    float boneRadius = (bone.boneLength.x + bone.boneLength.y + bone.boneLength.z) / 3.0f;
    boneRadius *= adaptiveScale; // Apply adaptive scaling (now 1.0 = no scaling)
    
    // Apply falloff multiplier to control metaball influence radius
    // Higher falloff = thicker/blended meshes, Lower falloff = thinner/defined meshes
    float effectiveRadius = boneRadius * m_falloffMultiplier;
    
    // Compute bone endpoint (growth direction)
    // Use pre-computed worldEndpoint for parent-child continuity
    DirectX::XMFLOAT3 boneEnd = bone.worldEndpoint;
    
    // If worldEndpoint not computed (fallback), calculate from boneLength
    if (boneEnd.x == 0.0f && boneEnd.y == 0.0f && boneEnd.z == 0.0f && bone.parentIndex != -1) {
        float maxX = std::abs(bone.boneLength.x);
        float maxY = std::abs(bone.boneLength.y);
        float maxZ = std::abs(bone.boneLength.z);
        
        if (maxY >= maxX && maxY >= maxZ)
            boneEnd = { bonePos.x, bonePos.y + bone.boneLength.y, bonePos.z };
        else if (maxX >= maxY && maxX >= maxZ)
            boneEnd = { bonePos.x + bone.boneLength.x, bonePos.y, bonePos.z };
        else
            boneEnd = { bonePos.x, bonePos.y, bonePos.z + bone.boneLength.z };
    }
    
    // Archetype-specific SDF primitive
    switch (archetype)
    {
        case ArchetypeType::Chordata:
            // Capsule SDF for vertebrate limbs (smooth muscle blending)
            return CapsuleSDF(voxelPos, bonePos, boneEnd, effectiveRadius);
            
        case ArchetypeType::Arthropoda:
            // Cylinder SDF for exoskeleton segments (use existing implementation)
            return CylinderSDF(voxelPos, bonePos, boneEnd, effectiveRadius, 1.0f);
            
        case ArchetypeType::Mollusca:
            // Soft metaball for hydrostatic skeleton
            // Reduced from 2.0x to 1.3x - joint spheres handle connectivity, not oversized bones
            DirectX::XMFLOAT3 center = {
                (bonePos.x + boneEnd.x) * 0.5f,
                (bonePos.y + boneEnd.y) * 0.5f,
                (bonePos.z + boneEnd.z) * 0.5f
            };
            return MetaballSDF(voxelPos, center, effectiveRadius * 1.3f);
            
        default:
            // Fallback to capsule
            return CapsuleSDF(voxelPos, bonePos, boneEnd, effectiveRadius);
    }
}

// Smooth union for SDF blending (Inigo Quilez polynomial version)
// Maintains true distance field properties (unlike linear summation)
float ScalarFieldGenerator::SmoothUnion(float d1, float d2, float k) const {
    float h = std::clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
    return std::lerp(d2, d1, h) - k * h * (1.0f - h);
}

// Cylinder signed distance function (kept for Arthropoda exoskeleton segments)
float ScalarFieldGenerator::CylinderSDF(
    const DirectX::XMFLOAT3& pos,
    const DirectX::XMFLOAT3& boneStart,
    const DirectX::XMFLOAT3& boneEnd,
    float radius,
    float adaptiveScale) const
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
    
    // Return true SDF (positive outside, negative inside)
    return distance - radius;
}

// Capsule SDF (from Inigo Quilez)
float ScalarFieldGenerator::CapsuleSDF(
    const DirectX::XMFLOAT3& pos,
    const DirectX::XMFLOAT3& p1,
    const DirectX::XMFLOAT3& p2,
    float radius) const
{
    DirectX::XMVECTOR pa = DirectX::XMLoadFloat3(&pos);
    DirectX::XMVECTOR v_p1 = DirectX::XMLoadFloat3(&p1);
    DirectX::XMVECTOR v_p2 = DirectX::XMLoadFloat3(&p2);
    DirectX::XMVECTOR ba = DirectX::XMVectorSubtract(v_p2, v_p1);
    DirectX::XMVECTOR pb = DirectX::XMVectorSubtract(pa, v_p1);
    
    float h = DirectX::XMVectorGetX(DirectX::XMVector3Dot(pb, ba)) / 
              DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(ba));
    h = std::clamp(h, 0.0f, 1.0f);
    
    DirectX::XMVECTOR closest = DirectX::XMVectorMultiplyAdd(ba, DirectX::XMVectorReplicate(h), v_p1);
    return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(pa, closest))) - radius;
}

// Metaball SDF (4th order polynomial from Spore)
float ScalarFieldGenerator::MetaballSDF(
    const DirectX::XMFLOAT3& pos,
    const DirectX::XMFLOAT3& center,
    float radius) const
{
    DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&pos);
    DirectX::XMVECTOR c = DirectX::XMLoadFloat3(&center);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(p, c);
    float distSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(diff));
    
    // 4th order polynomial (smoother derivatives than linear)
    // f(d) = radius² * (1 - d²/radius²)²
    // SDF approximation: radius - d (but with smooth falloff)
    float normalizedDistSq = distSq / (radius * radius);
    if (normalizedDistSq >= 1.0f) return std::sqrt(distSq) - radius; // Outside
    
    // Inside: use polynomial for smooth blending
    float metaballValue = (1.0f - normalizedDistSq) * (1.0f - normalizedDistSq);
    return -metaballValue * radius; // Negative = inside
}

} // namespace Generation
} // namespace Procedural
} // namespace Engine
