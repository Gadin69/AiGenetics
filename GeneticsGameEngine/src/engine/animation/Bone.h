#pragma once

#include <windows.h>
#include <DirectXMath.h>
#include <string>

namespace Engine {
namespace Animation {

// Bone structure for hierarchical skeletal system
struct Bone {
    std::string name;
    int32_t parentIndex;              // -1 for root bone
    DirectX::XMFLOAT3 localPosition;  // Position relative to parent
    DirectX::XMFLOAT3 localRotation;  // Euler angles relative to parent (radians)
    DirectX::XMFLOAT3 boneLength;     // Bone dimensions (length, width, height)
    float mass;                       // For physics simulation
    
    // Computed world-space transforms (updated each frame)
    DirectX::XMFLOAT4X4 localTransform;
    DirectX::XMFLOAT4X4 worldTransform;
    
    Bone() : parentIndex(-1), 
             localPosition{0.0f, 0.0f, 0.0f},
             localRotation{0.0f, 0.0f, 0.0f},
             boneLength{1.0f, 0.1f, 0.1f},
             mass(1.0f) 
    {
        // Initialize transforms to identity (zero out, then set diagonal)
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                localTransform.m[i][j] = (i == j) ? 1.0f : 0.0f;
        
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                worldTransform.m[i][j] = (i == j) ? 1.0f : 0.0f;
    }
};

} // namespace Animation
} // namespace Engine
