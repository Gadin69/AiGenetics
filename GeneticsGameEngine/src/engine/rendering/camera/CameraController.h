#pragma once

#include <DirectXMath.h>
#include <algorithm>
#include "CameraTypes.h"

namespace Engine {
namespace Rendering {

// Base camera controller implementation
class CameraController : public BaseCameraController {
private:
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;
    
    float m_fov;
    float m_nearPlane;
    float m_farPlane;
    
    // Cached matrices
    mutable DirectX::XMFLOAT4X4 m_viewMatrix;
    mutable DirectX::XMFLOAT4X4 m_projectionMatrix;
    mutable DirectX::XMFLOAT4X4 m_viewProjectionMatrix;
    
public:
    CameraController() : 
        m_position(0.0f, 0.0f, -5.0f),
        m_rotation(0.0f, 0.0f, 0.0f),
        m_fov(DirectX::XM_PI / 4.0f),
        m_nearPlane(0.1f),
        m_farPlane(1000.0f),
        m_matricesDirty(true) {}
    
    // Implementation of BaseCameraController interface
    void Update(float deltaTime) override {
        // Default update does nothing - subclasses implement specific behavior
        m_matricesDirty = true;
    }
    
    void SetPosition(const DirectX::XMFLOAT3& position) override {
        m_position = position;
        m_matricesDirty = true;
    }
    
    void SetRotation(const DirectX::XMFLOAT3& rotation) override {
        m_rotation = rotation;
        m_matricesDirty = true;
    }
    
    DirectX::XMMATRIX GetViewMatrix() const override {
        if (m_matricesDirty) {
            UpdateMatrices();
        }
        return DirectX::XMLoadFloat4x4(&m_viewMatrix);
    }
    
    DirectX::XMMATRIX GetProjectionMatrix() const override {
        if (m_matricesDirty) {
            UpdateMatrices();
        }
        return DirectX::XMLoadFloat4x4(&m_projectionMatrix);
    }
    
    DirectX::XMMATRIX GetViewProjectionMatrix() const override {
        if (m_matricesDirty) {
            UpdateMatrices();
        }
        return DirectX::XMLoadFloat4x4(&m_viewProjectionMatrix);
    }
    
    void SetFOV(float fov) override {
        m_fov = fov;
        m_matricesDirty = true;
    }

    void SetNearPlane(float nearPlane) override {
        m_nearPlane = nearPlane;
        m_matricesDirty = true;
    }

    void SetFarPlane(float farPlane) override {
        m_farPlane = farPlane;
        m_matricesDirty = true;
    }
    
    // Movement methods
    void Rotate(float yaw, float pitch) override {
        m_rotation.y += yaw;
        m_rotation.x += pitch;
        
        // Clamp pitch to avoid flipping
        float minPitch = -DirectX::XM_PIDIV2 + 0.01f;
        float maxPitch = DirectX::XM_PIDIV2 - 0.01f;
        if (m_rotation.x < minPitch) m_rotation.x = minPitch;
        if (m_rotation.x > maxPitch) m_rotation.x = maxPitch;
        
        m_matricesDirty = true;
    }
    
    void MoveForward(float distance) override {
        DirectX::XMFLOAT3 forward = GetForwardVector();
        m_position.x += forward.x * distance;
        m_position.y += forward.y * distance;
        m_position.z += forward.z * distance;
        m_matricesDirty = true;
    }
    
    void MoveRight(float distance) override {
        DirectX::XMFLOAT3 right = GetRightVector();
        m_position.x += right.x * distance;
        m_position.y += right.y * distance;
        m_position.z += right.z * distance;
        m_matricesDirty = true;
    }
    
    void MoveUp(float distance) override {
        DirectX::XMFLOAT3 up = GetUpVector();
        m_position.x += up.x * distance;
        m_position.y += up.y * distance;
        m_position.z += up.z * distance;
        m_matricesDirty = true;
    }

protected:
    mutable bool m_matricesDirty;
    
    DirectX::XMFLOAT3 GetForwardVector() const override {
        // Calculate forward vector from rotation
        float pitch = m_rotation.x;
        float yaw = m_rotation.y;
        
        DirectX::XMFLOAT3 forward;
        forward.x = cosf(pitch) * sinf(yaw);
        forward.y = sinf(pitch);
        forward.z = cosf(pitch) * cosf(yaw);
        
        return forward;
    }
    
    DirectX::XMFLOAT3 GetRightVector() const override {
        // Calculate right vector from rotation
        float pitch = m_rotation.x;
        float yaw = m_rotation.y;
        
        DirectX::XMFLOAT3 right;
        right.x = cosf(yaw);
        right.y = 0.0f;
        right.z = -sinf(yaw);
        
        return right;
    }
    
    DirectX::XMFLOAT3 GetUpVector() const override {
        // Calculate up vector as cross product of forward and right
        DirectX::XMFLOAT3 forward = GetForwardVector();
        DirectX::XMFLOAT3 right = GetRightVector();
        
        DirectX::XMFLOAT3 up;
        up.x = forward.y * right.z - forward.z * right.y;
        up.y = forward.z * right.x - forward.x * right.z;
        up.z = forward.x * right.y - forward.y * right.x;
        
        return up;
    }
    
    // Public accessors for derived classes
    const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
    const DirectX::XMFLOAT3& GetRotation() const { return m_rotation; }
    void SetMatricesDirty() { m_matricesDirty = true; }
    
private:
    void UpdateMatrices() const {
        // Create view matrix
        DirectX::XMFLOAT3 forward = GetForwardVector();
        DirectX::XMFLOAT3 up = GetUpVector();
        DirectX::XMStoreFloat4x4(
            &m_viewMatrix,
            DirectX::XMMatrixLookAtLH(
                DirectX::XMLoadFloat3(&m_position),
                DirectX::XMLoadFloat3(&forward),
                DirectX::XMLoadFloat3(&up)
            )
        );
        
        // Create projection matrix
        DirectX::XMStoreFloat4x4(
            &m_projectionMatrix,
            DirectX::XMMatrixPerspectiveFovLH(
                m_fov,
                16.0f / 9.0f, // Aspect ratio
                m_nearPlane,
                m_farPlane
            )
        );
        
        // Create view-projection matrix
        DirectX::XMMATRIX viewMatrix = DirectX::XMLoadFloat4x4(&m_viewMatrix);
        DirectX::XMMATRIX projectionMatrix = DirectX::XMLoadFloat4x4(&m_projectionMatrix);
        DirectX::XMMATRIX viewProjectionMatrix = viewMatrix * projectionMatrix;
        DirectX::XMStoreFloat4x4(&m_viewProjectionMatrix, viewProjectionMatrix);
        
        m_matricesDirty = false;
    }
};

} // namespace Rendering
} // namespace Engine