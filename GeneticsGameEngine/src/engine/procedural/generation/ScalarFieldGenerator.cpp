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
    
    // DEBUG: Print first 5 bone world positions and parent indices to understand hierarchy
    printf("  [DEBUG BoneTransforms] Total bones: %zu\n", bones.size());
    for (size_t i = 0; i < bones.size() && i < 5; i++)
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
    
    // Calculate adaptive scaling factor based on bone count
    // More bones = more density summation = need smaller individual contributions
    // Target: keep maximum density around 1.0-2.0 for proper isosurface extraction
    // Adjusted for 1.5x radius falloff (tighter blending needs higher base density)
    float boneCount = static_cast<float>(bones.size());
    float adaptiveScale = 3.0f / (boneCount * 0.5f); // Higher base for tighter falloff
    
    printf("  [DEBUG ScalarField] Bone count: %zu, Adaptive scale: %.3f\n", bones.size(), adaptiveScale);
    
    // NEW: Fill scalar field using smooth SDF union (not linear summation)
    float minDensity = 999.0f;
    float maxDensity = -999.0f;
    int aboveIsovalue = 0;
    int belowIsovalue = 0;
    
    // Get archetype blend smoothness from params
    float blendK = params.blendSmoothness; // 0.05=hard (Arthropoda), 0.3=smooth (Chordata), 0.5=very smooth (Mollusca)
    
    // Calculate bounding box size to normalize SDF values
    float boundingSize = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    float sdfScale = 2.0f / boundingSize; // Scale SDF so creature fills the grid properly
    
    for (int z = 0; z < sizeZ; ++z) {
        for (int y = 0; y < sizeY; ++y) {
            for (int x = 0; x < sizeX; ++x) {
                // Convert voxel coordinates to world space
                DirectX::XMFLOAT3 voxelPos(
                    x * voxelSize + offsetX,
                    y * voxelSize + offsetY,
                    z * voxelSize + offsetZ
                );
                
                // Compute SDF from all bones using SMOOTH UNION
                // Start with far-away distance (positive = outside)
                float sdf = 999.0f;
                for (const auto& bone : bones)
                {
                    float boneSDF = ComputeBoneSDF(voxelPos, bone, adaptiveScale, params.archetype);
                    sdf = SmoothUnion(sdf, boneSDF, blendK);
                }
                
                // Scale SDF to ensure it crosses the 0.5 isovalue
                // Positive SDF = outside (should be < 0.5 density)
                // Negative SDF = inside (should be > 0.5 density)
                sdf *= sdfScale;
                
                // Convert SDF to density convention (negative inside = solid)
                // Invert: negative SDF (inside) becomes positive density
                // Add offset to center the range around 0.5
                float density = -sdf + 0.5f;
                
                // Track density range
                minDensity = std::min(minDensity, density);
                maxDensity = std::max(maxDensity, density);
                if (density > 0.5f) aboveIsovalue++;
                else belowIsovalue++;
                
                // Store in scalar field
                grid.SetScalarField(x, y, z, density);
            }
        }
    }
    
    // Debug output
    printf("  [DEBUG ScalarField] Density range: [%.3f, %.3f]\n", minDensity, maxDensity);
    printf("  [DEBUG ScalarField] Voxels above isovalue(0.5): %d, below: %d (total: %d)\n", 
           aboveIsovalue, belowIsovalue, sizeX * sizeY * sizeZ);
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
    float boneRadius = (bone.boneLength.x + bone.boneLength.y + bone.boneLength.z) / 4.0f;
    boneRadius *= adaptiveScale; // Apply adaptive scaling
    
    // Apply falloff multiplier to control metaball influence radius
    // Higher falloff = thicker/blended meshes, Lower falloff = thinner/defined meshes
    float effectiveRadius = boneRadius * m_falloffMultiplier;
    
    // Compute bone endpoint (growth direction)
    DirectX::XMFLOAT3 boneEnd = bonePos;
    float maxX = std::abs(bone.boneLength.x);
    float maxY = std::abs(bone.boneLength.y);
    float maxZ = std::abs(bone.boneLength.z);
    
    if (maxY >= maxX && maxY >= maxZ)
        boneEnd = { bonePos.x, bonePos.y + bone.boneLength.y, bonePos.z };
    else if (maxX >= maxY && maxX >= maxZ)
        boneEnd = { bonePos.x + bone.boneLength.x, bonePos.y, bonePos.z };
    else
        boneEnd = { bonePos.x, bonePos.y, bonePos.z + bone.boneLength.z };
    
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
            // Soft metaball for hydrostatic skeleton (2x radius for blobby look)
            DirectX::XMFLOAT3 center = {
                (bonePos.x + boneEnd.x) * 0.5f,
                (bonePos.y + boneEnd.y) * 0.5f,
                (bonePos.z + boneEnd.z) * 0.5f
            };
            return MetaballSDF(voxelPos, center, effectiveRadius * 2.0f);
            
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
