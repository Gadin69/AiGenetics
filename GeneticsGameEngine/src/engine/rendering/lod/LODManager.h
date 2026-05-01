#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <unordered_map>
#include "LODLevel.h"
#include "LODTransition.h"

namespace Engine {
namespace Rendering {



// LOD manager for automatic level selection
class LODManager {
private:
    std::vector<std::unique_ptr<LODLevel>> m_lodLevels;
    std::unordered_map<std::string, std::vector<std::unique_ptr<LODLevel>>> m_lodPreset;
    
public:
    LODManager() = default;
    
    // Add LOD level to current preset
    void AddLODLevel(const std::string& presetName, float distance, int vertexCount, int indexCount) {
        auto lod = std::make_unique<LODLevel>(distance, vertexCount, indexCount);
        m_lodPreset[presetName].push_back(std::move(lod));
    }
    
    // Get optimal LOD level for given distance
    const LODLevel* GetLODLevel(const std::string& presetName, float distance) const {
        auto it = m_lodPreset.find(presetName);
        if (it == m_lodPreset.end()) return nullptr;
        
        // Find closest LOD level
        const LODLevel* bestLOD = nullptr;
        float minDistance = FLT_MAX;
        
        for (const auto& lod : it->second) {
            float distDiff = abs(lod->distance - distance);
            if (distDiff < minDistance) {
                minDistance = distDiff;
                bestLOD = lod.get();
            }
        }
        
        return bestLOD;
    }
    
    // Get LOD level index for given distance
    int GetLODLevelIndex(const std::string& presetName, float distance) const {
        auto it = m_lodPreset.find(presetName);
        if (it == m_lodPreset.end()) return 0;
        
        // Find first LOD level where distance is less than or equal to threshold
        for (size_t i = 0; i < it->second.size(); ++i) {
            if (distance <= it->second[i]->distance) {
                return static_cast<int>(i);
            }
        }
        
        return static_cast<int>(it->second.size()) - 1;
    }
};

} // namespace Rendering
} // namespace Engine