#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <string>
#include <memory>

namespace GeneticsGameEngine {
namespace Rendering {

// Material ID type
typedef uint32_t MaterialID;

// PBR material constant buffer structure (must be 256-byte aligned)
struct PBRMaterialConstants {
    DirectX::XMFLOAT3 albedo;          // RGB color
    float padding1;                     // Alignment padding
    float roughness;                     // Surface roughness (0-1)
    float metallic;                      // Metallic property (0-1)
    float ambientOcclusion;              // AO factor (0-1)
    float emissiveIntensity;             // Emissive strength
    DirectX::XMFLOAT3 emissive;          // Emissive color
    float padding2;                      // Alignment padding
};

// Represents a single PBR material with optional textures
struct PBRMaterial {
    std::string materialID;
    
    // Material parameters
    DirectX::XMFLOAT3 albedo;
    DirectX::XMFLOAT3 emissive;          // Emissive color
    float roughness;
    float metallic;
    float ambientOcclusion;
    float emissiveIntensity;
    
    // Optional textures (can be null if using constant values)
    Microsoft::WRL::ComPtr<ID3D12Resource> albedoTexture;
    Microsoft::WRL::ComPtr<ID3D12Resource> normalMap;
    Microsoft::WRL::ComPtr<ID3D12Resource> roughnessMetallicAO; // Packed RMA texture
    
    // DX12 constant buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;
    PBRMaterialConstants* pConstantData = nullptr;
    
    // Constructor
    PBRMaterial() 
        : albedo(1.0f, 1.0f, 1.0f)
        , emissive(0.0f, 0.0f, 0.0f)
        , roughness(0.5f)
        , metallic(0.0f)
        , ambientOcclusion(1.0f)
        , emissiveIntensity(0.0f)
    {}
    
    // Update constant buffer with current parameters
    void UpdateConstantBuffer();
    
    // Initialize constant buffer
    bool InitializeConstantBuffer(ID3D12Device* device);
    
    // Cleanup resources
    void Cleanup();
};

} // namespace Rendering
} // namespace GeneticsGameEngine
