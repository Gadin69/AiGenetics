#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <memory>

namespace GeneticsGameEngine {
namespace Rendering {

// HDR renderer manages HDR render targets and tone mapping
class HDRRenderer
{
public:
    HDRRenderer();
    ~HDRRenderer();
    
    bool Initialize(ID3D12Device* device, UINT width, UINT height, 
                    ID3D12CommandQueue* commandQueue, IDXGIFactory4* dxgiFactory);
    void Shutdown();
    
    // Resize HDR buffers
    void Resize(UINT width, UINT height);
    
    // Begin HDR frame
    void BeginFrame(ID3D12GraphicsCommandList* commandList);
    
    // End HDR frame (apply tone mapping and resolve to back buffer)
    void EndFrame(ID3D12GraphicsCommandList* commandList, ID3D12Resource* backBuffer);
    
    // Get HDR render target view
    D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;
    
    // Tone mapping mode
    enum class ToneMappingMode
    {
        None,
        Reinhard,
        ACES_Filmic,
        Uncharted2
    };
    
    void SetToneMappingMode(ToneMappingMode mode) { toneMappingMode_ = mode; }
    ToneMappingMode GetToneMappingMode() const { return toneMappingMode_; }
    
    // Exposure control
    void SetExposure(float exposure) { exposure_ = exposure; }
    float GetExposure() const { return exposure_; }
    
private:
    bool CreateHDRRenderTarget(UINT width, UINT height);
    bool CreateToneMappingResources();
    
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    
    // HDR render target
    Microsoft::WRL::ComPtr<ID3D12Resource> hdrRenderTarget_;
    Microsoft::WRL::ComPtr<ID3D12Resource> hdrDepthStencil_;
    D3D12_CPU_DESCRIPTOR_HANDLE hdrRTVHandle_;
    D3D12_CPU_DESCRIPTOR_HANDLE hdrDSVHandle_;
    
    // Tone mapping
    ToneMappingMode toneMappingMode_ = ToneMappingMode::ACES_Filmic;
    float exposure_ = 1.0f;
    
    UINT width_ = 0;
    UINT height_ = 0;
    
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
};

} // namespace Rendering
} // namespace GeneticsGameEngine
