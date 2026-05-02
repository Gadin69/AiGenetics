#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class GeneticsIntegration;

namespace Engine {
    namespace Rendering {
        class BaseCameraController;
    }
}

// Vertex structure for rendering
struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

// Camera constant buffer structure (must be 256-byte aligned)
struct CameraConstants {
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;
};

class GraphicsEngine
{
public:
    GraphicsEngine();
    ~GraphicsEngine();

    // Prevent copying
    GraphicsEngine(const GraphicsEngine&) = delete;
    GraphicsEngine& operator=(const GraphicsEngine&) = delete;

    // Initialization
    bool Initialize(HWND hWnd);
    void Cleanup();

    // Render loop
    void Render();
    void Render(std::unique_ptr<GeneticsIntegration>& geneticsIntegration, 
                Engine::Rendering::BaseCameraController* camera = nullptr);

    // Update
    void Update();

private:
    // Core DX12 objects (using ComPtr for automatic reference counting)
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    
    // Swap chain
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    
    // Descriptor heaps
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
    
    // Render targets
    static const UINT FrameCount = 2;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    UINT m_frameIndex = 0;
    
    // Depth/stencil buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
    
    // Descriptor sizes
    UINT m_rtvDescriptorSize = 0;
    UINT m_dsvDescriptorSize = 0;
    UINT m_cbvSrvUavDescriptorSize = 0;
    
    // Command allocators (one per frame for frame resources)
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
    
    // Vertex buffer (simple triangle for testing)
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
    
    // Ground plane (indexed)
    Microsoft::WRL::ComPtr<ID3D12Resource> m_groundVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_groundIndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_groundVertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW m_groundIndexBufferView = {};
    UINT m_groundIndexCount = 0;
    
    // Constant buffer for camera
    Microsoft::WRL::ComPtr<ID3D12Resource> m_cameraConstantBuffer;
    UINT8* m_pCameraConstantData = nullptr;
    
    // Pipeline state
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    
    // Shader blobs
    Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShaderBlob;
    
    // Synchronization
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount] = {};
    HANDLE m_fenceEvent = nullptr;
    
    // Window handle
    HWND m_hWnd = nullptr;
    UINT m_width = 800;
    UINT m_height = 600;
    
    // Initialization methods
    bool InitializeDX12();
    bool CreateSwapChain();
    bool CreateDescriptorHeaps();
    bool CreateDepthBuffer();
    bool CreateRenderTargetViews();
    bool CreateCommandObjects();
    bool CreateSyncObjects();
    bool CompileShaders();
    bool CreateRootSignature();
    bool CreatePipelineState();
    bool CreateVertexBuffer();
    bool CreateGroundPlane();
    bool CreateCameraConstantBuffer();
    
    // Render loop helpers
    void UpdateCameraConstantBuffer(Engine::Rendering::BaseCameraController* camera);
    void PopulateCommandList(Engine::Rendering::BaseCameraController* camera);
    void WaitForPreviousFrame();
    void MoveToNextFrame();
    
    // Utility
    void ThrowIfFailed(HRESULT hr, const std::string& message = "");
};
