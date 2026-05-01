#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>

namespace Engine {
namespace Rendering {

// LOD transition manager for smooth transitions
class LODTransition {
private:
    float m_transitionStartDistance;
    float m_transitionEndDistance;
    float m_transitionProgress;
    
public:
    LODTransition(float startDist = 10.0f, float endDist = 20.0f) : 
        m_transitionStartDistance(startDist), 
        m_transitionEndDistance(endDist), 
        m_transitionProgress(0.0f) {}
    
    // Calculate transition progress based on distance
    void UpdateTransition(float currentDistance) {
        if (currentDistance <= m_transitionStartDistance) {
            m_transitionProgress = 0.0f;
        } else if (currentDistance >= m_transitionEndDistance) {
            m_transitionProgress = 1.0f;
        } else {
            m_transitionProgress = (currentDistance - m_transitionStartDistance) / 
                                 (m_transitionEndDistance - m_transitionStartDistance);
        }
    }
    
    float GetTransitionProgress() const { return m_transitionProgress; }
    
    // Interpolate between two LOD levels
    template<typename T>
    T Interpolate(const T& level1, const T& level2) const {
        return static_cast<T>(level1 * (1.0f - m_transitionProgress) + level2 * m_transitionProgress);
    }
};

} // namespace Rendering
} // namespace Engine