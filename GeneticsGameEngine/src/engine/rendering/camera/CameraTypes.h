#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>

namespace Engine {
namespace Rendering {

// Base camera controller interface
class BaseCameraController {
public:
    virtual ~BaseCameraController() = default;
    
    // Camera state management
    virtual void Update(float deltaTime) = 0;
    virtual void SetPosition(const DirectX::XMFLOAT3& position) = 0;
    virtual void SetRotation(const DirectX::XMFLOAT3& rotation) = 0;
    
    // Camera matrix generation
    virtual DirectX::XMMATRIX GetViewMatrix() const = 0;
    virtual DirectX::XMMATRIX GetProjectionMatrix() const = 0;
    virtual DirectX::XMMATRIX GetViewProjectionMatrix() const = 0;
    
    // Camera properties
    virtual void SetFOV(float fov) = 0;
    virtual void SetNearPlane(float nearPlane) = 0;
    virtual void SetFarPlane(float farPlane) = 0;
    
    // Utility methods
    virtual DirectX::XMFLOAT3 GetForwardVector() const = 0;
    virtual DirectX::XMFLOAT3 GetRightVector() const = 0;
    virtual DirectX::XMFLOAT3 GetUpVector() const = 0;
};

// Camera types enum
class CameraType {
public:
    enum Type {
        ORBIT,
        FIRST_PERSON,
        CINEMATIC,
        STATIC
    };
};

} // namespace Rendering
} // namespace Engine