#pragma once

#include <vector>
#include <algorithm>
#include <DirectXMath.h>

namespace Engine {
namespace Procedural {
namespace Voxel {

// LOD configuration from tables.txt TABLE 9 (lines 545-558)
struct LODConfig {
    float lod0MaxDistance;    // 10.0f
    float lod1MaxDistance;    // 30.0f
    float lod2MaxDistance;    // 80.0f
    float transitionZone;     // 2.0f
    
    int lod0GridSize;         // 128
    int lod1GridSize;         // 64
    int lod2GridSize;         // 32
    
    float lod0TargetTriCount; // 50000
    float lod1TargetTriCount; // 10000
    float lod2TargetTriCount; // 2000
    
    LODConfig() :
        lod0MaxDistance(10.0f),
        lod1MaxDistance(30.0f),
        lod2MaxDistance(80.0f),
        transitionZone(2.0f),
        lod0GridSize(128),
        lod1GridSize(64),
        lod2GridSize(32),
        lod0TargetTriCount(50000),
        lod1TargetTriCount(10000),
        lod2TargetTriCount(2000) {}
};

// LOD manager for voxel-based creatures
// From tables.txt TABLE 9 (lines 524-543)
class VoxelLODManager {
private:
    LODConfig m_config;
    
public:
    VoxelLODManager() = default;
    VoxelLODManager(const LODConfig& config) : m_config(config) {}
    
    // Select LOD level based on distance to camera
    // From tables.txt TABLE 9 (lines 524-543):
    // LOD0: 0-10 units (128x128x128)
    // LOD1: 10-30 units (64x64x64)
    // LOD2: 30-80 units (32x32x32)
    // LOD3: 80+ units (bounding box)
    int SelectLODLevel(float distanceToCamera) const {
        if (distanceToCamera <= m_config.lod0MaxDistance) {
            return 0; // LOD0 - highest detail
        } else if (distanceToCamera <= m_config.lod1MaxDistance) {
            return 1; // LOD1 - medium detail
        } else if (distanceToCamera <= m_config.lod2MaxDistance) {
            return 2; // LOD2 - low detail
        } else {
            return 3; // LOD3 - ultra-low (bounding box)
        }
    }
    
    // Get grid size for LOD level
    int GetGridSizeForLOD(int lodLevel) const {
        switch (lodLevel) {
            case 0: return m_config.lod0GridSize;
            case 1: return m_config.lod1GridSize;
            case 2: return m_config.lod2GridSize;
            default: return m_config.lod2GridSize; // Use lowest for LOD3
        }
    }
    
    // Get target triangle count for LOD level
    float GetTargetTriCountForLOD(int lodLevel) const {
        switch (lodLevel) {
            case 0: return m_config.lod0TargetTriCount;
            case 1: return m_config.lod1TargetTriCount;
            case 2: return m_config.lod2TargetTriCount;
            default: return m_config.lod2TargetTriCount;
        }
    }
    
    // Calculate smooth LOD transition alpha
    // From tables.txt TABLE 9 (lines 532-534)
    float CalculateLODTransition(float distanceToCamera, int currentLOD) const {
        float threshold = 0.0f;
        switch (currentLOD) {
            case 0: threshold = m_config.lod0MaxDistance; break;
            case 1: threshold = m_config.lod1MaxDistance; break;
            case 2: threshold = m_config.lod2MaxDistance; break;
            default: return 1.0f;
        }
        
        float zone = m_config.transitionZone;
        float alpha = (distanceToCamera - (threshold - zone)) / (2.0f * zone);
        return std::clamp(alpha, 0.0f, 1.0f);
    }
    
    // Get LOD configuration
    const LODConfig& GetConfig() const { return m_config; }
    void SetConfig(const LODConfig& config) { m_config = config; }
};

} // namespace Voxel
} // namespace Procedural
} // namespace Engine
