#include "HDRRenderer.h"
#include <iostream>
#include <d3dcompiler.h>

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

using namespace GeneticsGameEngine::Rendering;

HDRRenderer::HDRRenderer()
{
}

HDRRenderer::~HDRRenderer()
{
    Shutdown();
}

bool HDRRenderer::Initialize(ID3D12Device* device, UINT width, UINT height,
                              ID3D12CommandQueue* commandQueue, IDXGIFactory4* dxgiFactory)
{
    if (!device || !commandQueue || !dxgiFactory)
    {
        std::cerr << "[HDRRenderer] Invalid device, command queue, or DXGI factory pointer" << std::endl;
        return false;
    }

    device_ = device;
    commandQueue_ = commandQueue;
    width_ = width;
    height_ = height;

    // Create descriptor heaps
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_))))
    {
        std::cerr << "[HDRRenderer] Failed to create RTV descriptor heap" << std::endl;
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap_))))
    {
        std::cerr << "[HDRRenderer] Failed to create DSV descriptor heap" << std::endl;
        return false;
    }

    // Get descriptor handles
    hdrRTVHandle_ = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    hdrDSVHandle_ = dsvHeap_->GetCPUDescriptorHandleForHeapStart();

    // Create HDR render target
    if (!CreateHDRRenderTarget(width, height))
    {
        std::cerr << "[HDRRenderer] Failed to create HDR render target" << std::endl;
        return false;
    }

    std::cout << "[HDRRenderer] Initialized (" << width << "x" << height << ")" << std::endl;
    return true;
}

void HDRRenderer::Shutdown()
{
    hdrRenderTarget_.Reset();
    hdrDepthStencil_.Reset();
    rtvHeap_.Reset();
    dsvHeap_.Reset();
    device_.Reset();
    commandQueue_.Reset();

    std::cout << "[HDRRenderer] Shutdown complete" << std::endl;
}

void HDRRenderer::Resize(UINT width, UINT height)
{
    if (width == width_ && height == height_)
        return;

    width_ = width;
    height_ = height;

    // Recreate HDR render target
    hdrRenderTarget_.Reset();
    hdrDepthStencil_.Reset();

    CreateHDRRenderTarget(width, height);

    std::cout << "[HDRRenderer] Resized to " << width << "x" << height << std::endl;
}

void HDRRenderer::BeginFrame(ID3D12GraphicsCommandList* commandList)
{
    if (!commandList || !hdrRenderTarget_)
        return;

    // Transition HDR render target to render target state
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = hdrRenderTarget_.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    commandList->ResourceBarrier(1, &barrier);

    // Clear HDR render target
    FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(hdrRTVHandle_, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(hdrDSVHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set HDR render target
    commandList->OMSetRenderTargets(1, &hdrRTVHandle_, FALSE, &hdrDSVHandle_);
}

void HDRRenderer::EndFrame(ID3D12GraphicsCommandList* commandList, ID3D12Resource* backBuffer)
{
    if (!commandList || !hdrRenderTarget_ || !backBuffer)
        return;

    // Transition HDR render target to shader resource
    D3D12_RESOURCE_BARRIER toSRBarrier = {};
    toSRBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    toSRBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    toSRBarrier.Transition.pResource = hdrRenderTarget_.Get();
    toSRBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    toSRBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    toSRBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

    commandList->ResourceBarrier(1, &toSRBarrier);

    // Copy HDR to back buffer (simplified - would normally use tone mapping shader)
    D3D12_RESOURCE_BARRIER toPresentBarrier = {};
    toPresentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    toPresentBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    toPresentBarrier.Transition.pResource = backBuffer;
    toPresentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    toPresentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    toPresentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

    commandList->ResourceBarrier(1, &toPresentBarrier);

    commandList->CopyResource(backBuffer, hdrRenderTarget_.Get());

    // Transition back buffer to present
    D3D12_RESOURCE_BARRIER toPresentBarrier2 = {};
    toPresentBarrier2.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    toPresentBarrier2.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    toPresentBarrier2.Transition.pResource = backBuffer;
    toPresentBarrier2.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    toPresentBarrier2.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    toPresentBarrier2.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    commandList->ResourceBarrier(1, &toPresentBarrier2);

    // Transition HDR render target back to render target
    D3D12_RESOURCE_BARRIER toRTBarrier = {};
    toRTBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    toRTBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    toRTBarrier.Transition.pResource = hdrRenderTarget_.Get();
    toRTBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    toRTBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    toRTBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    commandList->ResourceBarrier(1, &toRTBarrier);
}

D3D12_CPU_DESCRIPTOR_HANDLE HDRRenderer::GetRenderTargetView() const
{
    return hdrRTVHandle_;
}

bool HDRRenderer::CreateHDRRenderTarget(UINT width, UINT height)
{
    if (width == 0 || height == 0)
    {
        std::cerr << "[HDRRenderer] Invalid dimensions: " << width << "x" << height << std::endl;
        return false;
    }
    
    // Create HDR render target (FP16 format for HDR)
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 1.0f;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    HRESULT hr = device_->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearValue,
        IID_PPV_ARGS(&hdrRenderTarget_));
    
    if (FAILED(hr))
    {
        std::cerr << "[HDRRenderer] Failed to create HDR render target (HRESULT: 0x" 
                  << std::hex << hr << std::dec << ")" << std::endl;
        return false;
    }

    // Create RTV
    device_->CreateRenderTargetView(hdrRenderTarget_.Get(), nullptr, hdrRTVHandle_);

    // Create depth stencil
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthClear = {};
    depthClear.Format = DXGI_FORMAT_D32_FLOAT;
    depthClear.DepthStencil.Depth = 1.0f;
    depthClear.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES depthHeapProps(D3D12_HEAP_TYPE_DEFAULT);

    hr = device_->CreateCommittedResource(
        &depthHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClear,
        IID_PPV_ARGS(&hdrDepthStencil_));
    
    if (FAILED(hr))
    {
        std::cerr << "[HDRRenderer] Failed to create depth stencil (HRESULT: 0x" 
                  << std::hex << hr << std::dec << ")" << std::endl;
        return false;
    }

    // Create DSV
    device_->CreateDepthStencilView(hdrDepthStencil_.Get(), nullptr, hdrDSVHandle_);

    return true;
}

bool HDRRenderer::CreateToneMappingResources()
{
    // TODO: Create tone mapping pipeline state and shaders
    return true;
}
