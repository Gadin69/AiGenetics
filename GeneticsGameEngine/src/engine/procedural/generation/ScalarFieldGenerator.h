#pragma once

#include "CreatureParams.h"
#include "../voxel/VoxelGrid.h"
#include "../../animation/Skeleton.h"
#include "../../animation/Bone.h"
#include <DirectXMath.h>

namespace Engine {
namespace Procedural {
namespace Voxel {
    class VoxelGrid;  // Forward declaration
} // namespace Voxel

namespace Generation {

// Generates scalar field density functions for creature body shapes
// From tables.txt TABLE 8 (lines 478-516): Metaballs / Blobby Surfaces
class ScalarFieldGenerator {
public:
    ScalarFieldGenerator() = default;
    
    // Generate scalar field for entire voxel grid based on creature parameters
    void GenerateField(Voxel::VoxelGrid& grid, const CreatureParams& params);
    
    // NEW: Generate scalar field from skeleton (bones act as metaball attractors)
    void GenerateFieldFromSkeleton(Voxel::VoxelGrid& grid, 
                                    const Engine::Animation::Skeleton& skeleton,
                                    const CreatureParams& params);
    
private:
    // Main density function for creature shape
    // From tables.txt TABLE 8 (lines 491-512)
    float CreatureDensityFunction(const DirectX::XMFLOAT3& pos, const CreatureParams& params) const;
    
    // Helper functions
    float DistanceToCylinder(const DirectX::XMFLOAT3& pos, 
                            const DirectX::XMFLOAT3& start, 
                            const DirectX::XMFLOAT3& end, 
                            float radius) const;
    
    float Smoothstep(float edge0, float edge1, float x) const;
    
    float EllipsoidDensity(const DirectX::XMFLOAT3& pos, 
                          const DirectX::XMFLOAT3& center, 
                          const DirectX::XMFLOAT3& radii) const;
    
    float SphereDensity(const DirectX::XMFLOAT3& pos, 
                       const DirectX::XMFLOAT3& center, 
                       float radius) const;
    
    // NEW: Skeleton-based density functions
    float ComputeBoneDensity(const DirectX::XMFLOAT3& voxelPos, 
                             const Engine::Animation::Bone& bone) const;
    
    float CylinderSDF(const DirectX::XMFLOAT3& pos,
                      const DirectX::XMFLOAT3& boneStart,
                      const DirectX::XMFLOAT3& boneEnd,
                      float radius) const;
};

} // namespace Generation
} // namespace Procedural
} // namespace Engine
