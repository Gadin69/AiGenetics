#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "BoundingVolume.h"

namespace Engine {
namespace Rendering {

// Frustum culler for efficient object culling
class FrustumCuller {
private:
    // Frustum planes (left, right, bottom, top, near, far)
    DirectX::XMFLOAT4 m_planes[6];
    
public:
    FrustumCuller() = default;
    
    // Update frustum from view-projection matrix
    void UpdateFromViewProjection(const DirectX::XMFLOAT4X4& viewProjectionMatrix) {
        // Extract frustum planes from view-projection matrix
        // Left plane: column0 + column3
        m_planes[0].x = viewProjectionMatrix._11 + viewProjectionMatrix._41;
        m_planes[0].y = viewProjectionMatrix._12 + viewProjectionMatrix._42;
        m_planes[0].z = viewProjectionMatrix._13 + viewProjectionMatrix._43;
        m_planes[0].w = viewProjectionMatrix._14 + viewProjectionMatrix._44;
        
        // Right plane: column3 - column0
        m_planes[1].x = viewProjectionMatrix._41 - viewProjectionMatrix._11;
        m_planes[1].y = viewProjectionMatrix._42 - viewProjectionMatrix._12;
        m_planes[1].z = viewProjectionMatrix._43 - viewProjectionMatrix._13;
        m_planes[1].w = viewProjectionMatrix._44 - viewProjectionMatrix._14;
        
        // Bottom plane: column2 + column3
        m_planes[2].x = viewProjectionMatrix._31 + viewProjectionMatrix._41;
        m_planes[2].y = viewProjectionMatrix._32 + viewProjectionMatrix._42;
        m_planes[2].z = viewProjectionMatrix._33 + viewProjectionMatrix._43;
        m_planes[2].w = viewProjectionMatrix._34 + viewProjectionMatrix._44;
        
        // Top plane: column3 - column2
        m_planes[3].x = viewProjectionMatrix._41 - viewProjectionMatrix._31;
        m_planes[3].y = viewProjectionMatrix._42 - viewProjectionMatrix._32;
        m_planes[3].z = viewProjectionMatrix._43 - viewProjectionMatrix._33;
        m_planes[3].w = viewProjectionMatrix._44 - viewProjectionMatrix._34;
        
        // Near plane: column2 + column3
        m_planes[4].x = viewProjectionMatrix._21 + viewProjectionMatrix._41;
        m_planes[4].y = viewProjectionMatrix._22 + viewProjectionMatrix._42;
        m_planes[4].z = viewProjectionMatrix._23 + viewProjectionMatrix._43;
        m_planes[4].w = viewProjectionMatrix._24 + viewProjectionMatrix._44;
        
        // Far plane: column3 - column2
        m_planes[5].x = viewProjectionMatrix._41 - viewProjectionMatrix._21;
        m_planes[5].y = viewProjectionMatrix._42 - viewProjectionMatrix._22;
        m_planes[5].z = viewProjectionMatrix._43 - viewProjectionMatrix._23;
        m_planes[5].w = viewProjectionMatrix._44 - viewProjectionMatrix._24;
        
        // Normalize planes
        for (int i = 0; i < 6; ++i) {
            float length = sqrtf(m_planes[i].x * m_planes[i].x + 
                               m_planes[i].y * m_planes[i].y + 
                               m_planes[i].z * m_planes[i].z);
            if (length > 0.0f) {
                m_planes[i].x /= length;
                m_planes[i].y /= length;
                m_planes[i].z /= length;
                m_planes[i].w /= length;
            }
        }
    }
    
    // Test if bounding volume intersects frustum
    template<typename T>
    bool Intersects(const T& volume) const {
        return volume.Intersects(m_planes);
    }
    
    // Get frustum planes array
    const DirectX::XMFLOAT4* GetPlanes() const {
        return m_planes;
    }
    
    // Check if point is inside frustum
    bool ContainsPoint(const DirectX::XMFLOAT3& point) const {
        for (int i = 0; i < 6; ++i) {
            float distance = m_planes[i].x * point.x + 
                           m_planes[i].y * point.y + 
                           m_planes[i].z * point.z + 
                           m_planes[i].w;
            if (distance < 0.0f) {
                return false;
            }
        }
        return true;
    }
    
    // Check if sphere intersects frustum
    bool IntersectsSphere(const DirectX::XMFLOAT3& center, float radius) const {
        float distance;
        for (int i = 0; i < 6; ++i) {
            distance = m_planes[i].x * center.x + 
                      m_planes[i].y * center.y + 
                      m_planes[i].z * center.z + 
                      m_planes[i].w;
            
            if (distance < -radius) {
                return false;
            }
            
            if (distance > radius) {
                continue;
            }
        }
        return true;
    }
};

} // namespace Rendering
} // namespace Engine