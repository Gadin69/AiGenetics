#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>

namespace Engine {
namespace Rendering {

// Bounding volume for frustum culling
class BoundingVolume {
public:
    enum Type {
        SPHERE,
        AABB,
        OBB
    };
    
    virtual ~BoundingVolume() = default;
    virtual bool Intersects(const DirectX::XMFLOAT4* frustumPlanes) const = 0;
    virtual Type GetType() const = 0;
};

// Bounding sphere for simple culling
class BoundingSphere : public BoundingVolume {
private:
    DirectX::XMFLOAT3 m_center;
    float m_radius;
    
public:
    BoundingSphere(const DirectX::XMFLOAT3& center, float radius) : 
        m_center(center), m_radius(radius) {}
    
    bool Intersects(const DirectX::XMFLOAT4* frustumPlanes) const override {
        // Check if sphere intersects each frustum plane
        for (int i = 0; i < 6; ++i) {
            float distance = frustumPlanes[i].x * m_center.x + 
                           frustumPlanes[i].y * m_center.y + 
                           frustumPlanes[i].z * m_center.z + 
                           frustumPlanes[i].w;
            
            if (distance < -m_radius) {
                return false;
            }
        }
        return true;
    }
    
    Type GetType() const override { return SPHERE; }
};

} // namespace Rendering
} // namespace Engine