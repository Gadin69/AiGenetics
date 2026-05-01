#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <windows.h>
#include <vector>
#include <memory>
#include <DirectXMath.h>

// Forward declarations
class GeneticsIntegration;

// Forward declarations for engine classes to avoid circular dependencies
namespace Engine {
    namespace Rendering {
        class BaseCameraController;
        class FrustumCuller;
        class SpatialPartition;
        class LODManager;
        class ProjectionMatrix;
    }
}

// Creature rendering data structure
struct CreatureRenderData {
    float scale;
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
    
    CreatureRenderData() 
        : scale(1.0f), 
          position(0.0f, 0.0f, 0.0f), 
          color(1.0f, 1.0f, 1.0f, 1.0f) {}
};

class GraphicsEngine
{
private:
    HWND m_hWnd;
    
    // DirectX 12 objects
    ID3D12Device* m_device;
    IDXGIAdapter* m_adapter;
    IDXGIFactory4* m_factory;
    ID3D12CommandQueue* m_commandQueue;
    ID3D12CommandAllocator* m_commandAllocator;
    ID3D12GraphicsCommandList* m_commandList;
    
    // Swap chain
    IDXGISwapChain* m_swapChain;
    IDXGISwapChain3* m_swapChain3;
    ID3D12Resource* m_renderTargets[2];
    UINT m_frameIndex;
    
    // RTV resources
    ID3D12DescriptorHeap* m_rtvHeap;
    UINT m_rtvDescriptorSize;
    
    // Vertex buffer resources
    ID3D12Resource* m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    
    // Instance buffer resources for creature rendering
    ID3D12Resource* m_instanceBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_instanceBufferView;
    UINT m_maxInstanceCount;
    
    // Pipeline state object
    ID3D12PipelineState* m_pipelineState;
    
    // Root signature
    ID3D12RootSignature* m_rootSignature;
    
    // Shader blobs
    ID3DBlob* m_vertexShaderBlob;
    ID3DBlob* m_pixelShaderBlob;
    
    // Synchronization
    HANDLE m_fenceEvent;
    ID3D12Fence* m_fence;
    UINT64 m_fenceValue;
    
public:
    GraphicsEngine() : m_hWnd(nullptr), m_device(nullptr), m_adapter(nullptr), 
        m_factory(nullptr), m_commandQueue(nullptr), m_commandAllocator(nullptr), 
        m_commandList(nullptr), m_swapChain(nullptr), m_swapChain3(nullptr), m_frameIndex(0), 
        m_rtvHeap(nullptr), m_rtvDescriptorSize(0), m_vertexBuffer(nullptr), 
        m_instanceBuffer(nullptr), m_maxInstanceCount(0), m_pipelineState(nullptr), 
        m_rootSignature(nullptr), m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr), 
        m_fenceEvent(nullptr), m_fence(nullptr), m_fenceValue(0) {}
    
    bool Initialize(HWND hWnd);
    void Cleanup();
    
    void Update();
    void Render();
    void Render(std::unique_ptr<GeneticsIntegration>& geneticsIntegration, Engine::Rendering::BaseCameraController* camera = nullptr);
    
    // Render creatures with instanced rendering
    void RenderCreatures(const std::vector<CreatureRenderData>& creatures, Engine::Rendering::BaseCameraController* camera = nullptr);
    
private:
    bool InitializeDX12();
    bool CreateSwapChain();
    bool CreateCommandObjects();
    bool CreateSyncObjects();
    
    bool CreateRootSignature();
    bool CreatePipelineState();
    bool CreateVertexBuffer();
    
    bool CompileShaders();
    
    void WaitForPreviousFrame();
    void MoveToNextFrame();
};