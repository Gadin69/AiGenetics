#pragma once

#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
#include <iostream>
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
        
        // No pitch clamping - allow unrestricted rotation
        
        m_matricesDirty = true;
    }
    
    void MoveForward(float distance) override {
        DirectX::XMFLOAT3 forward = GetForwardVector();
        std::cout << "[CAMERA] MoveForward(" << distance << ") - forward vector: (" 
                  << forward.x << ", " << forward.y << ", " << forward.z << ")" << std::endl;
        std::cout << "[CAMERA] Position BEFORE: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
        m_position.x += forward.x * distance;
        m_position.y += forward.y * distance;
        m_position.z += forward.z * distance;
        std::cout << "[CAMERA] Position AFTER: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
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
    
    void SetWindowSize(float width, float height) {
        m_windowSize = DirectX::XMFLOAT2(width, height);
        m_matricesDirty = true;
    }

protected:
    mutable bool m_matricesDirty;
    DirectX::XMFLOAT2 m_windowSize;
    
    DirectX::XMFLOAT3 GetForwardVector() const override {
        // CAMERA MOVEMENT - DO NOT MODIFY unless explicitly requested
        // Calculates forward direction from pitch and yaw rotation angles
        // Uses left-handed coordinate system: forward = -Z when rotation is (0,0,0)
        float pitch = m_rotation.x;
        float yaw = m_rotation.y;
        
        DirectX::XMFLOAT3 forward;
        forward.x = cosf(pitch) * sinf(yaw);
        forward.y = sinf(pitch);
        forward.z = -cosf(pitch) * cosf(yaw);  // Negative Z for forward in LH coordinate system
        
        return forward;
    }
    
    DirectX::XMFLOAT3 GetRightVector() const override {
        // CAMERA MOVEMENT - DO NOT MODIFY unless explicitly requested
        // Calculates right vector as cross product of world-up and forward direction
        // This ensures stable strafing movement at all yaw angles (no gimbal lock)
        DirectX::XMFLOAT3 forward = GetForwardVector();
        DirectX::XMFLOAT3 worldUp = { 0.0f, 1.0f, 0.0f };
        
        // Cross product: right = worldUp × forward
        DirectX::XMFLOAT3 right;
        right.x = worldUp.y * forward.z - worldUp.z * forward.y;
        right.y = worldUp.z * forward.x - worldUp.x * forward.z;
        right.z = worldUp.x * forward.y - worldUp.y * forward.x;
        
        // Normalize the right vector
        float length = sqrtf(right.x * right.x + right.y * right.y + right.z * right.z);
        if (length > 0.001f) {
            right.x /= length;
            right.y /= length;
            right.z /= length;
        }
        
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
        // CAMERA MOVEMENT - DO NOT MODIFY unless explicitly requested
        // Creates view matrix using world-up vector (0,1,0) to prevent roll/gimbal lock
        std::cout << "[CAMERA] UpdateMatrices called - position: (" 
                  << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
        
        // Create view matrix
        DirectX::XMFLOAT3 forward = GetForwardVector();
        
        // FIXED: Always use world-up vector (0, 1, 0) for first-person camera
        // This prevents the view from rolling/flipping at horizontal rotation extremes
        DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
        
        // Calculate look-at target position (camera position + forward direction)
        DirectX::XMFLOAT3 lookAtTarget;
        lookAtTarget.x = m_position.x + forward.x;
        lookAtTarget.y = m_position.y + forward.y;
        lookAtTarget.z = m_position.z + forward.z;
        
        DirectX::XMStoreFloat4x4(
            &m_viewMatrix,
            DirectX::XMMatrixLookAtLH(
                DirectX::XMLoadFloat3(&m_position),
                DirectX::XMLoadFloat3(&lookAtTarget),
                DirectX::XMLoadFloat3(&up)
            )
        );
        
        // Create projection matrix with correct aspect ratio
        float aspectRatio = 16.0f / 9.0f; // Default fallback
        if (m_windowSize.x > 0.0f && m_windowSize.y > 0.0f) {
            aspectRatio = m_windowSize.x / m_windowSize.y;
        }
        
        DirectX::XMStoreFloat4x4(
            &m_projectionMatrix,
            DirectX::XMMatrixPerspectiveFovLH(
                m_fov,
                aspectRatio,
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