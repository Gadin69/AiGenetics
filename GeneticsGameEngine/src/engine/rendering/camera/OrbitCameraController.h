#pragma once

#include "CameraController.h"
#include <DirectXMath.h>

namespace Engine {
namespace Rendering {

// Orbit camera controller for scene inspection
class OrbitCameraController : public CameraController {
private:
    DirectX::XMFLOAT3 m_target;
    float m_distance;
    
public:
    OrbitCameraController() : 
        m_target(0.0f, 0.0f, 0.0f),
        m_distance(5.0f) {}
    
    void SetTarget(const DirectX::XMFLOAT3& target) {
        m_target = target;
        m_matricesDirty = true;
    }
    
    void SetDistance(float distance) {
        m_distance = distance;
        m_matricesDirty = true;
    }
    
    float GetDistance() const {
        return m_distance;
    }
    
    // Override Update to implement orbit behavior
    void Update(float deltaTime) override {
        // Calculate position based on rotation and target
        // Yaw (Y rotation) = horizontal orbit around Y axis
        // Pitch (X rotation) = vertical angle from horizon
        float yaw = GetRotation().y;
        float pitch = GetRotation().x;
            
        DirectX::XMFLOAT3 position;
        position.x = m_target.x + m_distance * cosf(pitch) * sinf(yaw);
        position.y = m_target.y + m_distance * sinf(pitch);
        position.z = m_target.z + m_distance * cosf(pitch) * cosf(yaw);
            
        SetPosition(position);
            
        // Ensure matrices are updated
        SetMatricesDirty();
    }
    
    // Override GetForwardVector to point toward target
    DirectX::XMFLOAT3 GetForwardVector() const override {
        DirectX::XMFLOAT3 forward;
        forward.x = m_target.x - GetPosition().x;
        forward.y = m_target.y - GetPosition().y;
        forward.z = m_target.z - GetPosition().z;
            
        // Normalize
        float length = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
        if (length > 0.0f) {
            forward.x /= length;
            forward.y /= length;
            forward.z /= length;
        }
            
        return forward;
    }
};

} // namespace Rendering
} // namespace Engine