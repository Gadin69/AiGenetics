#include "PBRMaterial.h"
#include <DirectXMath.h>

// D3D12 Helper structures
struct CD3DX12_HEAP_PROPERTIES : public D3D12_HEAP_PROPERTIES
{
    CD3DX12_HEAP_PROPERTIES() = default;
    explicit CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE type)
    {
        Type = type;
        CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        CreationNodeMask = 1;
        VisibleNodeMask = 1;
    }
};

struct CD3DX12_RESOURCE_DESC : public D3D12_RESOURCE_DESC
{
    CD3DX12_RESOURCE_DESC() = default;
    static inline CD3DX12_RESOURCE_DESC Buffer(UINT64 size, UINT64 alignment = 0)
    {
        CD3DX12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = alignment;
        desc.Width = size;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        return desc;
    }
};

namespace GeneticsGameEngine {
namespace Rendering {

using namespace DirectX;

bool PBRMaterial::InitializeConstantBuffer(ID3D12Device* device)
{
    // Create constant buffer (256-byte aligned)
    const UINT bufferSize = (sizeof(PBRMaterialConstants) + 255) & ~255;
    
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    
    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constantBuffer));
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Map the constant buffer
    hr = constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pConstantData));
    if (FAILED(hr)) {
        return false;
    }
    
    // Initialize with current values
    UpdateConstantBuffer();
    
    return true;
}

void PBRMaterial::UpdateConstantBuffer()
{
    if (pConstantData) {
        pConstantData->albedo = albedo;
        pConstantData->padding1 = 0.0f;
        pConstantData->roughness = roughness;
        pConstantData->metallic = metallic;
        pConstantData->ambientOcclusion = ambientOcclusion;
        pConstantData->emissiveIntensity = emissiveIntensity;
        pConstantData->emissive = emissive;
        pConstantData->padding2 = 0.0f;
    }
}

void PBRMaterial::Cleanup()
{
    if (constantBuffer && pConstantData) {
        constantBuffer->Unmap(0, nullptr);
        pConstantData = nullptr;
    }
    
    albedoTexture.Reset();
    normalMap.Reset();
    roughnessMetallicAO.Reset();
    constantBuffer.Reset();
}

} // namespace Rendering
} // namespace GeneticsGameEngine
