#pragma once

#include <windows.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

namespace Engine {
namespace Animation {

// Structural connection types (non-hierarchical skeleton edges for mesh generation)
enum class StructuralConnectionType : uint8_t {
    NONE = 0,
    RIB,            // Vertebra to sternum
    WEBBING,        // Finger bones to membrane
    SHELL_BRIDGE,   // Segment to segment (Arthropoda)
    RADIAL,         // Tentacle to core mass (Mollusca)
    MUSCLE          // Soft tissue connection
};

// Bone structure for hierarchical skeletal system
struct Bone {
    std::string name;
    int32_t parentIndex;              // -1 for root bone
    DirectX::XMFLOAT3 localPosition;  // Position relative to parent
    DirectX::XMFLOAT3 localRotation;  // Euler angles relative to parent (radians)
    DirectX::XMFLOAT3 boneLength;     // Bone dimensions (length, width, height)
    float mass;                       // For physics simulation
    
    // NEW: Local anatomical frame (for proper voxel growth direction)
    DirectX::XMFLOAT3 forwardAxis;  // Primary growth direction (anterior)
    DirectX::XMFLOAT3 upAxis;       // Dorsal direction
    DirectX::XMFLOAT3 rightAxis;    // Lateral direction
    
    // NEW: Structural connections (for mesh generation, not animation)
    std::vector<int32_t> structuralConnections; // Indices of connected bones
    std::vector<StructuralConnectionType> connectionTypes; // Type of each connection
    
    // Computed world-space transforms (updated each frame)
    DirectX::XMFLOAT4X4 localTransform;
    DirectX::XMFLOAT4X4 worldTransform;
    DirectX::XMFLOAT3 worldEndpoint; // Where this bone ends in world space (for SDF continuity)
    
    Bone() : parentIndex(-1), 
             localPosition{0.0f, 0.0f, 0.0f},
             localRotation{0.0f, 0.0f, 0.0f},
             boneLength{1.0f, 0.1f, 0.1f},
             mass(1.0f),
             forwardAxis{0.0f, 0.0f, 1.0f},  // Default: +Z is forward
             upAxis{0.0f, 1.0f, 0.0f},       // Default: +Y is up
             rightAxis{1.0f, 0.0f, 0.0f},    // Default: +X is right
             worldEndpoint{0.0f, 0.0f, 0.0f}
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
