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
struct CreatureMeshData;

namespace Engine {
    namespace Rendering {
        class BaseCameraController;
    }
}

// PBR System includes
#include "../engine/rendering/materials/pbr/MaterialSystem.h"
#include "../engine/rendering/HDRRenderer.h"

using namespace GeneticsGameEngine::Rendering;

// Vertex structure for rendering
struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT4 color;
};

// Camera constant buffer structure (must be 256-byte aligned)
struct CameraConstants {
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;
};

// Light constant buffer structure (must be 256-byte aligned)
struct alignas(256) LightConstants {
    DirectX::XMFLOAT3 sunDirection;      // Normalized sun direction
    float sunIntensity;                   // Sun intensity multiplier
    
    DirectX::XMFLOAT3 sunColor;          // Sun color (RGB)
    float ambientIntensity;              // Ambient light intensity
    
    DirectX::XMFLOAT3 ambientColor;      // Ambient light color (sky contribution)
    float groundAmbientIntensity;         // Ground bounce light intensity
    
    DirectX::XMFLOAT3 groundAmbientColor; // Ground reflection color
    float pad0;                           // Alignment padding
};

// Time of Day configuration
struct TimeOfDayConfig {
    float sunAngle;                       // Sun angle in radians (0 = sunrise, PI/2 = noon)
    DirectX::XMFLOAT3 sunColor;          // Sun color at this TOD
    DirectX::XMFLOAT3 skyZenithColor;    // Sky color at zenith (top)
    DirectX::XMFLOAT3 skyHorizonColor;   // Sky color at horizon
    DirectX::XMFLOAT3 groundColor;       // Ground ambient color
    float ambientIntensity;               // Ambient intensity multiplier
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
    
    // Getters for Phase 3 integration
    ID3D12Device* GetDevice() const { return m_device.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }
    
    // Wireframe toggle
    void ToggleWireframe() { m_wireframeMode = !m_wireframeMode; }
    bool IsWireframeMode() const { return m_wireframeMode; }
    
    // Mouse input handling for UI
    void OnMouseMove(int x, int y);
    void OnMouseLeave();
    void OnMouseClick(int x, int y);
    bool IsPointInButton(int x, int y) const;

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
    
    // Wireframe rendering state
    bool m_wireframeMode = false;
    
    // UI button state (for clickable wireframe toggle)
    struct UIButton {
        float x, y, width, height;
        std::wstring text;
        bool hovered;
        bool pressed;
    };
    UIButton m_wireframeButton;
    bool m_mouseInWindow = false;
    int m_mouseX = 0;
    int m_mouseY = 0;
    
    // Pipeline state
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_wireframePipelineState;  // Wireframe for basic rendering
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    
    // Shader blobs
    Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShaderBlob;
    
    // PBR Shader blobs
    Microsoft::WRL::ComPtr<ID3DBlob> m_pbrVertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> m_pbrPixelShaderBlob;
    
    // PBR pipeline state
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pbrPipelineState;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pbrWireframePipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pbrRootSignature;
    
    // PBR Material System
    std::unique_ptr<MaterialSystem> m_materialSystem;
    
    // HDR Renderer
    std::unique_ptr<HDRRenderer> m_hdrRenderer;
    
    // Lighting system
    Microsoft::WRL::ComPtr<ID3D12Resource> m_lightConstantBuffer;
    LightConstants m_lightData = {};
    UINT8* m_lightCBVData = nullptr;
    
    // Sky dome
    Microsoft::WRL::ComPtr<ID3D12Resource> m_skyDomeVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_skyDomeIndexBuffer;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_skyDomePipelineState;
    D3D12_VERTEX_BUFFER_VIEW m_skyDomeVertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW m_skyDomeIndexBufferView = {};
    UINT m_skyDomeVertexCount = 0;
    UINT m_skyDomeIndexCount = 0;
    
    // Shader blobs for sky dome
    Microsoft::WRL::ComPtr<ID3DBlob> m_skyDomeVertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> m_skyDomePixelShaderBlob;
    
    // TOD system
    TimeOfDayConfig m_todConfig;
    float m_currentTimeOfDay = 0.25f; // 0.0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset
    
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
    bool CreateWireframePipelineState();  // Wireframe PSO for basic rendering
    bool CreateVertexBuffer();
    bool CreateGroundPlane();
    bool CreateCameraConstantBuffer();
    
    // PBR initialization methods
    bool CompilePBRShaders();
    bool CreatePBRRootSignature();
    bool CreatePBRPipelineState();
    bool CreatePBRWireframePipelineState();
    bool InitializePBRSystem();
    
    // Lighting and sky dome methods
    bool CreateLightConstantBuffer();
    bool CreateSkyDome();
    bool CreateSkyDomePipelineState();
    void UpdateLightingFromTOD();
    void RenderSkyDome();
    
    // UI rendering
    void InitializeUIButton();
    void CheckButtonHover();
    
    // Render loop helpers
    void UpdateCameraConstantBuffer(Engine::Rendering::BaseCameraController* camera);
    void PopulateCommandList(Engine::Rendering::BaseCameraController* camera, const std::vector<CreatureMeshData>& creatures);
    void RenderCreatures(const std::vector<CreatureMeshData>& creatures, Engine::Rendering::BaseCameraController* camera);
    void WaitForPreviousFrame();
    void MoveToNextFrame();
    
    // Utility
    void ThrowIfFailed(HRESULT hr, const std::string& message = "");
};
