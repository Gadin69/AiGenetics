#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>

namespace Engine {
namespace Rendering {

// LOD level definition with mesh data
struct LODLevel {
    float distance;
    int vertexCount;
    int indexCount;
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<unsigned short> indices;
    
    LODLevel(float dist, int vCount, int iCount) : 
        distance(dist), vertexCount(vCount), indexCount(iCount) {}
    
    // Calculate screen coverage percentage for this LOD level
    float CalculateScreenCoverage(const DirectX::XMMATRIX& viewProjection, 
                                const DirectX::XMFLOAT3& position, 
                                float screenWidth, float screenHeight) const {
        // Transform position to clip space
        DirectX::XMFLOAT4 clipPos;
        DirectX::XMStoreFloat4(&clipPos, DirectX::XMVector4Transform(DirectX::XMLoadFloat3(&position), viewProjection));
        
        if (clipPos.w == 0.0f) return 0.0f;
        
        // Normalize to NDC
        float ndcX = clipPos.x / clipPos.w;
        float ndcY = clipPos.y / clipPos.w;
        
        // Convert to screen space
        float screenX = (ndcX + 1.0f) * 0.5f * screenWidth;
        float screenY = (1.0f - ndcY) * 0.5f * screenHeight;
        
        // Simple approximation: use bounding sphere radius
        float radius = 1.0f; // This would be calculated from actual mesh bounds
        float screenRadius = radius * (screenWidth / 2.0f) / clipPos.w;
        
        return (screenRadius * screenRadius) / (screenWidth * screenHeight);
    }
};



} // namespace Rendering
} // namespace Engine