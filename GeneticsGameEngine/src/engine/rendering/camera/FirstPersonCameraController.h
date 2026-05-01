#pragma once

#include "CameraController.h"
#include <DirectXMath.h>

namespace Engine {
namespace Rendering {

// First-person camera controller for gameplay
class FirstPersonCameraController : public CameraController {
private:
    float m_sensitivity;
    bool m_isMoving;
    
public:
    FirstPersonCameraController() : 
        m_sensitivity(0.1f),
        m_isMoving(false) {}
    
    void SetSensitivity(float sensitivity) {
        m_sensitivity = sensitivity;
    }
    
    // Override Update to implement first-person behavior
    void Update(float deltaTime) override {
        // Handle mouse input for rotation
        // This would be connected to input system in actual implementation
        // For now, we'll just update matrices
        m_matricesDirty = true;
    }
    
    // Handle movement input
    void MoveForward(float amount) {
        DirectX::XMFLOAT3 forward = GetForwardVector();
        auto position = GetPosition();
        position.x += forward.x * amount;
        position.y += forward.y * amount;
        position.z += forward.z * amount;
        SetPosition(position);
        SetMatricesDirty();
    }
    
    void MoveRight(float amount) {
        DirectX::XMFLOAT3 right = GetRightVector();
        auto position = GetPosition();
        position.x += right.x * amount;
        position.y += right.y * amount;
        position.z += right.z * amount;
        SetPosition(position);
        SetMatricesDirty();
    }
    
    void MoveUp(float amount) {
        DirectX::XMFLOAT3 up = GetUpVector();
        auto position = GetPosition();
        position.x += up.x * amount;
        position.y += up.y * amount;
        position.z += up.z * amount;
        SetPosition(position);
        SetMatricesDirty();
    }
};

} // namespace Rendering
} // namespace Engine