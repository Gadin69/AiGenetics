#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>

namespace Engine {
namespace Rendering {

// Projection matrix types
class ProjectionMatrix {
public:
    enum Type {
        PERSPECTIVE,
        ORTHOGRAPHIC,
        CUSTOM
    };
    
private:
    DirectX::XMFLOAT4X4 m_matrix;
    Type m_type;
    
public:
    ProjectionMatrix(Type type = PERSPECTIVE) : m_type(type) {
        Reset();
    }
    
    void Reset() {
        // Identity matrix
        m_matrix._11 = 1.0f; m_matrix._12 = 0.0f; m_matrix._13 = 0.0f; m_matrix._14 = 0.0f;
        m_matrix._21 = 0.0f; m_matrix._22 = 1.0f; m_matrix._23 = 0.0f; m_matrix._24 = 0.0f;
        m_matrix._31 = 0.0f; m_matrix._32 = 0.0f; m_matrix._33 = 1.0f; m_matrix._34 = 0.0f;
        m_matrix._41 = 0.0f; m_matrix._42 = 0.0f; m_matrix._43 = 0.0f; m_matrix._44 = 1.0f;
    }
    
    void SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
        m_type = PERSPECTIVE;
        DirectX::XMStoreFloat4x4(&m_matrix, 
            DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane));
    }
    
    void SetOrthographic(float width, float height, float nearPlane, float farPlane) {
        m_type = ORTHOGRAPHIC;
        DirectX::XMStoreFloat4x4(&m_matrix, 
            DirectX::XMMatrixOrthographicLH(width, height, nearPlane, farPlane));
    }
    
    void SetCustom(const DirectX::XMFLOAT4X4& matrix) {
        m_type = CUSTOM;
        m_matrix = matrix;
    }
    
    const DirectX::XMFLOAT4X4& GetMatrix() const { return m_matrix; }
    Type GetType() const { return m_type; }
};

} // namespace Rendering
} // namespace Engine