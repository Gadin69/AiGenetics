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
        // First-person camera just needs matrices updated
        m_matricesDirty = true;
    }
};

} // namespace Rendering
} // namespace Engine