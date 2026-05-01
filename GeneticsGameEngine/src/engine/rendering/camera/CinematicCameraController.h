#pragma once

#include "CameraController.h"
#include <DirectXMath.h>
#include <vector>

namespace Engine {
namespace Rendering {

// Cinematic camera controller for cutscenes
class CinematicCameraController : public CameraController {
private:
    std::vector<DirectX::XMFLOAT3> m_keyframes;
    std::vector<float> m_times;
    float m_currentTime;
    int m_currentKeyframe;
    
public:
    CinematicCameraController() : 
        m_currentTime(0.0f),
        m_currentKeyframe(0) {}
    
    // Add keyframe to cinematic path
    void AddKeyframe(const DirectX::XMFLOAT3& position, float time) {
        m_keyframes.push_back(position);
        m_times.push_back(time);
    }
    
    // Override Update to implement cinematic movement
    void Update(float deltaTime) override {
        m_currentTime += deltaTime;
        
        // Simple linear interpolation between keyframes
        if (m_keyframes.size() >= 2 && m_times.size() >= 2) {
            // Find current segment
            for (int i = 0; i < static_cast<int>(m_times.size()) - 1; ++i) {
                if (m_currentTime >= m_times[i] && m_currentTime <= m_times[i + 1]) {
                    float t = (m_currentTime - m_times[i]) / (m_times[i + 1] - m_times[i]);
                    
                    DirectX::XMFLOAT3 pos1 = m_keyframes[i];
                    DirectX::XMFLOAT3 pos2 = m_keyframes[i + 1];
                    
                    auto position = GetPosition();
                    position.x = pos1.x + t * (pos2.x - pos1.x);
                    position.y = pos1.y + t * (pos2.y - pos1.y);
                    position.z = pos1.z + t * (pos2.z - pos1.z);
                    SetPosition(position);
                    SetMatricesDirty();
                    break;
                }
            }
        }
    }
    
    // Reset cinematic playback
    void ResetPlayback() {
        m_currentTime = 0.0f;
        m_currentKeyframe = 0;
    }
};

} // namespace Rendering
} // namespace Engine