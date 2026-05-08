#include "GraphicsEngine.h"
#include "../engine/rendering/camera/CameraController.h"
#include "../engine/ui/ImGuiRenderer.h"
#include "../genetics/GeneticsIntegration.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <d3d12sdklayers.h>

// Define missing constant
#ifndef D3DCOMPILE_STANDARD_FILE_INCLUDE
#define D3DCOMPILE_STANDARD_FILE_INCLUDE nullptr
#endif

// Include handler for shader compilation
class ShaderIncludeHandler : public ID3DInclude
{
public:
    STDMETHOD(Open)(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData,
                    LPCVOID* ppData, UINT* pBytes) override
    {
        // Read the included file
        std::ifstream file(pFileName, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            return E_FAIL;
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        char* buffer = new char[size];
        if (!file.read(buffer, size))
        {
            delete[] buffer;
            return E_FAIL;
        }
        
        *ppData = buffer;
        *pBytes = static_cast<UINT>(size);
        return S_OK;
    }
    
    STDMETHOD(Close)(LPCVOID pData) override
    {
        delete[] static_cast<const char*>(pData);
        return S_OK;
    }
};

// D3D12 Helper structures (minimal implementation of d3dx12.h)
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


struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE() = default;
    explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE& other) 
    { 
        ptr = other.ptr; 
    }
    
    CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_CPU_DESCRIPTOR_HANDLE base, UINT offset, UINT descriptorSize)
    {
        ptr = base.ptr + offset * descriptorSize;
    }
    
    inline void Offset(UINT offset, UINT descriptorSize)
    {
        ptr += offset * descriptorSize;
    }
};

struct CD3DX12_RANGE : public D3D12_RANGE
{
    CD3DX12_RANGE() = default;
    CD3DX12_RANGE(SIZE_T begin, SIZE_T end) : D3D12_RANGE{ begin, end } {}
};

// Constructor
GraphicsEngine::GraphicsEngine()
{
}

// Destructor
GraphicsEngine::~GraphicsEngine()
{
    Cleanup();
}

// Utility function for error checking
void GraphicsEngine::ThrowIfFailed(HRESULT hr, const std::string& message)
{
    if (FAILED(hr))
    {
        std::cerr << "DirectX Error: " << message 
                  << " (HRESULT: 0x" << std::hex << hr << std::dec << ")" << std::endl;
        throw std::runtime_error(message);
    }
}

// Main initialization
bool GraphicsEngine::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;
    
    // Get window dimensions
    RECT rect;
    GetClientRect(hWnd, &rect);
    m_width = rect.right - rect.left;
    m_height = rect.bottom - rect.top;
    
    std::cout << "Initializing DirectX 12 (Complete Rewrite)..." << std::endl;
    
    try
    {
        // Step 1: Initialize DX12 device and debug layer
        if (!InitializeDX12())
        {
            std::cerr << "Failed to initialize DirectX 12 device" << std::endl;
            return false;
        }
        
        // Step 2: Create command objects
        if (!CreateCommandObjects())
        {
            std::cerr << "Failed to create command objects" << std::endl;
            return false;
        }
        
        // Step 3: Create swap chain
        if (!CreateSwapChain())
        {
            std::cerr << "Failed to create swap chain" << std::endl;
            return false;
        }
        
        // Step 4: Create descriptor heaps
        if (!CreateDescriptorHeaps())
        {
            std::cerr << "Failed to create descriptor heaps" << std::endl;
            return false;
        }
        
        // Step 5: Create depth/stencil buffer
        if (!CreateDepthBuffer())
        {
            std::cerr << "Failed to create depth buffer" << std::endl;
            return false;
        }
        
        // Step 5.5: Create shadow map
        if (!CreateShadowMap())
        {
            std::cerr << "Failed to create shadow map" << std::endl;
            return false;
        }
        
        // Step 6: Create render target views
        if (!CreateRenderTargetViews())
        {
            std::cerr << "Failed to create render target views" << std::endl;
            return false;
        }
        
        // Step 7: Create synchronization objects
        if (!CreateSyncObjects())
        {
            std::cerr << "Failed to create sync objects" << std::endl;
            return false;
        }
        
        // Step 8: Compile shaders
        if (!CompileShaders())
        {
            std::cerr << "Failed to compile shaders" << std::endl;
            return false;
        }
        
        // Step 9: Create root signature
        if (!CreateRootSignature())
        {
            std::cerr << "Failed to create root signature" << std::endl;
            return false;
        }
        
        // Step 10: Create pipeline state object
        if (!CreatePipelineState())
        {
            std::cerr << "Failed to create pipeline state" << std::endl;
            return false;
        }
        
        // Step 10.5: Create wireframe pipeline state for basic rendering
        if (!CreateWireframePipelineState())
        {
            std::cerr << "Failed to create wireframe pipeline state" << std::endl;
            return false;
        }
        
        // Step 11: Create vertex buffer (test triangle)
        if (!CreateVertexBuffer())
        {
            std::cerr << "Failed to create vertex buffer" << std::endl;
            return false;
        }
        
        // Step 12: Create ground plane
        if (!CreateGroundPlane())
        {
            std::cerr << "Failed to create ground plane" << std::endl;
            return false;
        }
        
        // Step 13: Create camera constant buffer
        if (!CreateCameraConstantBuffer())
        {
            std::cerr << "Failed to create camera constant buffer" << std::endl;
            return false;
        }
        
        // Step 14: Initialize PBR system
        if (!InitializePBRSystem())
        {
            std::cerr << "Failed to initialize PBR system" << std::endl;
            return false;
        }
        
        // Step 15: Initialize ImGui
        m_imguiRenderer = std::make_unique<ImGuiRenderer>();
        if (!m_imguiRenderer->Initialize(
            m_device.Get(),
            m_commandQueue.Get(),
            m_rtvHeap.Get(),
            m_dsvHeap.Get(),
            m_swapChain.Get(),
            FrameCount,
            m_width,
            m_height))
        {
            std::cerr << "Failed to initialize ImGui" << std::endl;
            return false;
        }
        
        std::cout << "DirectX 12 initialization completed successfully!" << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception during initialization: " << e.what() << std::endl;
        return false;
    }
}

// Cleanup
void GraphicsEngine::Cleanup()
{
    // Close fence event handle
    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
    
    // ComPtr objects will automatically release when destroyed
}

// Update (placeholder)
void GraphicsEngine::Update()
{
    // Update logic here
}

// Simple render
void GraphicsEngine::Render()
{
    std::unique_ptr<GeneticsIntegration> emptyIntegration;
    Render(emptyIntegration, nullptr);
}

// Complete render with genetics integration and camera
void GraphicsEngine::Render(std::unique_ptr<GeneticsIntegration>& geneticsIntegration,
                            Engine::Rendering::BaseCameraController* camera)
{
    static int frameCount = 0;
    
    // Debug: Print frame start when wireframe mode changes
    if (m_wireframeMode && frameCount < 10) {
        std::cout << "[RENDER] Starting frame " << frameCount << " with WIREFRAME MODE ON" << std::endl;
        std::cout.flush();
    }
    
    try
    {
        // Wait for GPU to finish previous frame
        if (m_wireframeMode && frameCount < 10) {
            std::cout << "[RENDER] Waiting for previous frame..." << std::endl;
            std::cout.flush();
        }
        WaitForPreviousFrame();
        
        // Start new ImGui frame
        if (m_imguiRenderer)
        {
            m_imguiRenderer->NewFrame();
            
            // Demo window for testing
            ImGui::ShowDemoWindow();
        }
        
        // Get creature meshes
        const auto& creatures = geneticsIntegration->GetCreatureMeshes();
        
        // Populate command list (including creature rendering)
        if (m_wireframeMode && frameCount < 10) {
            std::cout << "[RENDER] Populating command list..." << std::endl;
            std::cout.flush();
        }
        PopulateCommandList(camera, creatures);
        
        // Execute command list
        if (m_wireframeMode && frameCount < 10) {
            std::cout << "[RENDER] Executing command list..." << std::endl;
            std::cout.flush();
        }
        
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(1, ppCommandLists);
        
        // Present frame
        if (m_wireframeMode && frameCount < 10) {
            std::cout << "[RENDER] Presenting frame..." << std::endl;
            std::cout.flush();
        }
        HRESULT hr = m_swapChain->Present(0, 0);
        if (FAILED(hr))
        {
            std::cerr << "Present failed with HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
        }
        
        // Move to next frame
        if (m_wireframeMode && frameCount < 10) {
            std::cout << "[RENDER] Moving to next frame..." << std::endl;
            std::cout.flush();
        }
        MoveToNextFrame();
        
        if (frameCount < 5)
        {
            std::cout << "Frame " << frameCount << " rendered successfully" << std::endl;
        }
        frameCount++;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[RENDER EXCEPTION] Standard exception in Render: " << e.what() << std::endl;
        std::cerr.flush();
    }
    catch (...)
    {
        std::cerr << "[RENDER CRITICAL] Unknown exception in Render - D3D12 crash likely" << std::endl;
        std::cerr.flush();
    }
}

// ============================================================================
// INITIALIZATION METHODS
// ============================================================================

bool GraphicsEngine::InitializeDX12()
{
    std::cout << "  Enabling D3D12 Debug Layer..." << std::endl;
    
    // Enable debug layer
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        std::cout << "  Debug Layer enabled" << std::endl;
    }
    
    // DISABLED: GPU-based validation causes performance issues and unexpected termination
    // Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
    // if (SUCCEEDED(debugController.As(&debugController1)))
    // {
    //     debugController1->SetEnableGPUBasedValidation(TRUE);
    //     std::cout << "  GPU-Based Validation enabled" << std::endl;
    // }
    
    // DISABLED: Break on error causes silent termination
    // Microsoft::WRL::ComPtr<ID3D12Debug2> debugController2;
    // if (SUCCEEDED(debugController.As(&debugController2)))
    // {
    //     debugController2->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
    //     debugController2->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    //     std::cout << "  Debug break on error enabled" << std::endl;
    // }
#endif
    
    // Create DXGI factory
    std::cout << "  Creating DXGI Factory..." << std::endl;
    UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
    dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    
    ThrowIfFailed(
        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)),
        "CreateDXGIFactory2 failed"
    );
    
    // Find adapter
    std::cout << "  Finding GPU adapter..." << std::endl;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    
    for (UINT adapterIndex = 0; 
         DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapters1(adapterIndex, &adapter); 
         ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        
        // Skip software adapters
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;
        
        // Check for D3D12 support
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, 
                                         _uuidof(ID3D12Device), nullptr)))
        {
            std::wcout << L"  Using adapter: " << desc.Description << std::endl;
            break;
        }
    }
    
    // Create device
    std::cout << "  Creating D3D12 Device..." << std::endl;
    ThrowIfFailed(
        D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)),
        "D3D12CreateDevice failed"
    );
    
    std::cout << "  Device created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateCommandObjects()
{
    std::cout << "  Creating command queue..." << std::endl;
    
    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    
    ThrowIfFailed(
        m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)),
        "CreateCommandQueue failed"
    );
    
    // Create command allocators (one per frame)
    std::cout << "  Creating command allocators..." << std::endl;
    for (UINT i = 0; i < FrameCount; ++i)
    {
        ThrowIfFailed(
            m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                             IID_PPV_ARGS(&m_commandAllocators[i])),
            "CreateCommandAllocator failed"
        );
    }
    
    // Create command list
    std::cout << "  Creating command list..." << std::endl;
    ThrowIfFailed(
        m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                    m_commandAllocators[0].Get(), nullptr,
                                    IID_PPV_ARGS(&m_commandList)),
        "CreateCommandList failed"
    );
    
    // Close command list initially (required before first use)
    m_commandList->Close();
    
    std::cout << "  Command objects created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateSwapChain()
{
    std::cout << "  Creating swap chain..." << std::endl;
    std::cout << "    Window size: " << m_width << "x" << m_height << std::endl;
    
    // Describe swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 }; // 1 sample, quality 0
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = 0;
    
    // Create swap chain
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(
        m_factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),
            m_hWnd,
            &swapChainDesc,
            nullptr, // No fullscreen desc
            nullptr, // No restrict to output
            &swapChain1
        ),
        "CreateSwapChainForHwnd failed"
    );
    
    // Get IDXGISwapChain3 interface
    ThrowIfFailed(
        swapChain1.As(&m_swapChain),
        "Failed to get IDXGISwapChain3"
    );
    
    // Get initial frame index
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    
    std::cout << "  Swap chain created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateDescriptorHeaps()
{
    std::cout << "  Creating descriptor heaps..." << std::endl;
    
    // RTV Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    ThrowIfFailed(
        m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)),
        "Create RTV heap failed"
    );
    
    // DSV Heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    ThrowIfFailed(
        m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)),
        "Create DSV heap failed"
    );
    
    // CBV/SRV/UAV Heap (shader visible)
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = 1; // 1 for camera CBV
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    
    ThrowIfFailed(
        m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvSrvUavHeap)),
        "Create CBV heap failed"
    );
    
    // Get descriptor sizes
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    std::cout << "    RTV descriptor size: " << m_rtvDescriptorSize << " bytes" << std::endl;
    std::cout << "    DSV descriptor size: " << m_dsvDescriptorSize << " bytes" << std::endl;
    std::cout << "    CBV descriptor size: " << m_cbvSrvUavDescriptorSize << " bytes" << std::endl;
    
    std::cout << "  Descriptor heaps created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateDepthBuffer()
{
    std::cout << "  Creating depth/stencil buffer..." << std::endl;
    
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Alignment = 0;
    depthDesc.Width = m_width;
    depthDesc.Height = m_height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc = { 1, 0 };
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil = { 1.0f, 0 };
    
    ThrowIfFailed(
        m_device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&m_depthStencilBuffer)
        ),
        "Create depth buffer failed"
    );
    
    // Create DSV
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    
    m_device->CreateDepthStencilView(
        m_depthStencilBuffer.Get(),
        &dsvDesc,
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );
    
    std::cout << "  Depth/stencil buffer created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateShadowMap()
{
    std::cout << "  Creating shadow map (" << ShadowMapSize << "x" << ShadowMapSize << ")..." << std::endl;
    
    // Create shadow map depth texture
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    
    D3D12_RESOURCE_DESC shadowDesc = {};
    shadowDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    shadowDesc.Alignment = 0;
    shadowDesc.Width = ShadowMapSize;
    shadowDesc.Height = ShadowMapSize;
    shadowDesc.DepthOrArraySize = 1;
    shadowDesc.MipLevels = 1;
    shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;  // Typeless for both DSV and SRV
    shadowDesc.SampleDesc.Count = 1;
    shadowDesc.SampleDesc.Quality = 0;
    shadowDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    shadowDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil = { 1.0f, 0 };
    
    ThrowIfFailed(
        m_device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &shadowDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&m_shadowMapTexture)
        ),
        "Create shadow map texture failed"
    );
    
    // Create SRV descriptor heap for shadow map
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    
    ThrowIfFailed(
        m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_shadowMapSrvHeap)),
        "Create shadow map SRV heap failed"
    );
    
    // Create SRV for shadow map
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;  // Read as float in shader
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    
    m_device->CreateShaderResourceView(
        m_shadowMapTexture.Get(),
        &srvDesc,
        m_shadowMapSrvHeap->GetCPUDescriptorHandleForHeapStart()
    );
    
    // Create DSV descriptor heap for shadow map (separate from main scene DSV)
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    ThrowIfFailed(
        m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_shadowMapDsvHeap)),
        "Create shadow map DSV heap failed"
    );
    
    // Create DSV for shadow map
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    
    m_device->CreateDepthStencilView(
        m_shadowMapTexture.Get(),
        &dsvDesc,
        m_shadowMapDsvHeap->GetCPUDescriptorHandleForHeapStart()
    );
    
    // Create shadow map constant buffer
    D3D12_RESOURCE_DESC cbDesc = {};
    cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    cbDesc.Width = 256;  // Two 4x4 matrices = 128 bytes, but need 256 alignment
    cbDesc.Height = 1;
    cbDesc.DepthOrArraySize = 1;
    cbDesc.MipLevels = 1;
    cbDesc.SampleDesc.Count = 1;
    cbDesc.SampleDesc.Quality = 0;
    cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;  // Changed from UNKNOWN
    cbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
    D3D12_HEAP_PROPERTIES cbHeapProps = {};
    cbHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    cbHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    cbHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    
    ThrowIfFailed(
        m_device->CreateCommittedResource(
            &cbHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &cbDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_shadowMapConstantBuffer)
        ),
        "Create shadow map constant buffer failed"
    );
    
    // Map constant buffer
    D3D12_RANGE readRange = { 0, 0 };  // No read range for upload heap
    ThrowIfFailed(
        m_shadowMapConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_shadowCBVData)),
        "Map shadow map constant buffer failed"
    );
    
    std::cout << "  Shadow map created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateRenderTargetViews()
{
    std::cout << "  Creating render target views..." << std::endl;
    
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart()
    );
    
    for (UINT i = 0; i < FrameCount; ++i)
    {
        // Get back buffer
        ThrowIfFailed(
            m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])),
            "GetBuffer failed"
        );
        
        // Create RTV
        m_device->CreateRenderTargetView(
            m_renderTargets[i].Get(),
            nullptr,
            rtvHandle
        );
        
        // Move to next descriptor
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }
    
    std::cout << "  Render target views created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateSyncObjects()
{
    std::cout << "  Creating synchronization objects..." << std::endl;
    
    // Create fence
    ThrowIfFailed(
        m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)),
        "CreateFence failed"
    );
    
    // Create fence event
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent)
    {
        std::cerr << "Failed to create fence event" << std::endl;
        return false;
    }
    
    // Initialize fence values
    for (UINT i = 0; i < FrameCount; ++i)
    {
        m_fenceValues[i] = 0;
    }
    
    std::cout << "  Synchronization objects created successfully" << std::endl;
    return true;
}

// ============================================================================
// SHADER & PIPELINE METHODS
// ============================================================================

bool GraphicsEngine::CompileShaders()
{
    std::cout << "  Compiling shaders..." << std::endl;
    
    // Compile vertex shader
    Microsoft::WRL::ComPtr<ID3DBlob> vertexError;
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    
    HRESULT hr = D3DCompileFromFile(
        L"vertex.hlsl",
        nullptr,
        D3DCOMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "vs_5_1",
        compileFlags,
        0,
        &m_vertexShaderBlob,
        &vertexError
    );
    
    if (FAILED(hr))
    {
        if (vertexError)
        {
            std::cerr << "Vertex shader compilation error: " 
                      << (char*)vertexError->GetBufferPointer() << std::endl;
        }
        return false;
    }
    
    std::cout << "  Vertex shader compiled successfully" << std::endl;
    
    // Compile pixel shader
    Microsoft::WRL::ComPtr<ID3DBlob> pixelError;
    hr = D3DCompileFromFile(
        L"pixel.hlsl",
        nullptr,
        D3DCOMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "ps_5_1",
        compileFlags,
        0,
        &m_pixelShaderBlob,
        &pixelError
    );
    
    if (FAILED(hr))
    {
        if (pixelError)
        {
            std::cerr << "Pixel shader compilation error: " 
                      << (char*)pixelError->GetBufferPointer() << std::endl;
        }
        return false;
    }
    
    std::cout << "  Pixel shader compiled successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateRootSignature()
{
    std::cout << "  Creating root signature..." << std::endl;
    
    // Use root CBV directly instead of descriptor table
    D3D12_ROOT_PARAMETER rootParam = {};
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParam.Descriptor.ShaderRegister = 0;  // register(b0)
    rootParam.Descriptor.RegisterSpace = 0;
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    
    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.NumParameters = 1;
    rootDesc.pParameters = &rootParam;
    rootDesc.NumStaticSamplers = 0;
    rootDesc.pStaticSamplers = nullptr;
    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    
    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    
    HRESULT hr = D3D12SerializeRootSignature(
        &rootDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error
    );
    
    if (FAILED(hr))
    {
        if (error)
        {
            std::cerr << "Root signature serialization error: " 
                      << (char*)error->GetBufferPointer() << std::endl;
        }
        return false;
    }
    
    ThrowIfFailed(
        m_device->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)
        ),
        "CreateRootSignature failed"
    );
    
    std::cout << "  Root signature created successfully (with root CBV)" << std::endl;
    return true;
}

bool GraphicsEngine::CreatePipelineState()
{
    std::cout << "  Creating pipeline state object..." << std::endl;
    
    // Define input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    
    // Describe and create graphics pipeline state object (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { 
        reinterpret_cast<UINT8*>(m_vertexShaderBlob->GetBufferPointer()), 
        m_vertexShaderBlob->GetBufferSize() 
    };
    psoDesc.PS = { 
        reinterpret_cast<UINT8*>(m_pixelShaderBlob->GetBufferPointer()), 
        m_pixelShaderBlob->GetBufferSize() 
    };
    
    // Rasterizer state - SOLID mode (default)
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    // Depth/stencil state - ENABLED for correct depth sorting
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    
    // Blend state
    D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
    rtBlendDesc.BlendEnable = FALSE;
    rtBlendDesc.LogicOpEnable = FALSE;
    rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    
    psoDesc.BlendState.RenderTarget[0] = rtBlendDesc;
    
    // Other state
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc = { 1, 0 };
    
    ThrowIfFailed(
        m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)),
        "CreateGraphicsPipelineState failed"
    );
    
    std::cout << "  Pipeline state object created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateWireframePipelineState()
{
    std::cout << "  Creating wireframe pipeline state object..." << std::endl;
    
    // Define input layout (same as solid PSO)
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    
    // Describe and create wireframe pipeline state object
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { 
        reinterpret_cast<UINT8*>(m_vertexShaderBlob->GetBufferPointer()), 
        m_vertexShaderBlob->GetBufferSize() 
    };
    psoDesc.PS = { 
        reinterpret_cast<UINT8*>(m_pixelShaderBlob->GetBufferPointer()), 
        m_pixelShaderBlob->GetBufferSize() 
    };
    
    // Rasterizer state - WIREFRAME mode
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    // Depth/stencil state - ENABLED (same as solid PSO)
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    
    // Blend state (same as solid PSO)
    D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
    rtBlendDesc.BlendEnable = FALSE;
    rtBlendDesc.LogicOpEnable = FALSE;
    rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    
    psoDesc.BlendState.RenderTarget[0] = rtBlendDesc;
    
    // Other state
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc = { 1, 0 };
    
    ThrowIfFailed(
        m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_wireframePipelineState)),
        "Create Wireframe PipelineState failed"
    );
    
    std::cout << "  Wireframe pipeline state created successfully" << std::endl;
    return true;
}

// ============================================================================
// GEOMETRY METHODS
// ============================================================================

bool GraphicsEngine::CreateVertexBuffer()
{
    std::cout << "  Creating vertex buffer (3D pyramid)..." << std::endl;
    
    // Define 3D pyramid vertices (4 faces, 12 vertices for proper normals)
    // Consistent counter-clockwise winding for outward normals (when viewed from outside)
    // Apex at TOP (Y=+1), base at BOTTOM (Y=-1)
    Vertex vertices[] =
    {
        // Front face (looking from +Z) - CCW: Top, Bottom-Right, Bottom-Left
        { { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.707f, 0.707f }, { 1.0f, 0.0f, 0.0f, 1.0f } },   // Apex TOP - Red
        { { 1.0f, -1.0f, 1.0f }, { 0.0f, 0.707f, 0.707f }, { 0.0f, 0.0f, 1.0f, 1.0f } },   // Bottom Right - Blue
        { { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.707f, 0.707f }, { 0.0f, 1.0f, 0.0f, 1.0f } },  // Bottom Left - Green
        
        // Right face (looking from +X) - CCW: Top, Bottom-Back, Bottom-Front
        { { 0.0f, 1.0f, 0.0f }, { 0.707f, 0.707f, 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },    // Apex TOP - Yellow
        { { 1.0f, -1.0f, -1.0f }, { 0.707f, 0.707f, 0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },  // Bottom Back - Cyan
        { { 1.0f, -1.0f, 1.0f }, { 0.707f, 0.707f, 0.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },   // Bottom Front - Magenta
        
        // Back face (looking from -Z) - CCW: Top, Bottom-Left, Bottom-Right
        { { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.707f, -0.707f }, { 1.0f, 0.5f, 0.0f, 1.0f } },    // Apex TOP - Orange
        { { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.707f, -0.707f }, { 0.0f, 0.5f, 1.0f, 1.0f } }, // Bottom Left - Sky Blue
        { { 1.0f, -1.0f, -1.0f }, { 0.0f, 0.707f, -0.707f }, { 0.5f, 1.0f, 0.0f, 1.0f } },  // Bottom Right - Lime
        
        // Left face (looking from -X) - CCW: Top, Bottom-Front, Bottom-Back
        { { 0.0f, 1.0f, 0.0f }, { -0.707f, 0.707f, 0.0f }, { 0.5f, 0.0f, 1.0f, 1.0f } },    // Apex TOP - Purple
        { { -1.0f, -1.0f, 1.0f }, { -0.707f, 0.707f, 0.0f }, { 0.0f, 1.0f, 0.5f, 1.0f } },  // Bottom Front - Mint
        { { -1.0f, -1.0f, -1.0f }, { -0.707f, 0.707f, 0.0f }, { 1.0f, 0.0f, 0.5f, 1.0f } }  // Bottom Back - Pink
    };
    
    const UINT64 bufferSize = sizeof(vertices);
    
    // Debug: Print first vertex position
    std::cout << "  [DEBUG] First pyramid vertex: (" 
              << vertices[0].position.x << ", " 
              << vertices[0].position.y << ", " 
              << vertices[0].position.z << ")" << std::endl;
    std::cout << "  [DEBUG] Total pyramid vertices: " << (bufferSize / sizeof(Vertex)) << std::endl;
    
    // Create upload heap
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    
    ThrowIfFailed(
        m_device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)
        ),
        "Create vertex buffer failed"
    );
    
    // Copy data to vertex buffer
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on CPU
    
    ThrowIfFailed(
        m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)),
        "Map vertex buffer failed"
    );
    
    memcpy(pVertexDataBegin, vertices, sizeof(vertices));
    m_vertexBuffer->Unmap(0, nullptr);
    
    // Create vertex buffer view
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = bufferSize;
    
    std::cout << "  Vertex buffer created: 3 vertices, " << bufferSize << " bytes" << std::endl;
    return true;
}

bool GraphicsEngine::CreateGroundPlane()
{
    std::cout << "  Creating ground plane..." << std::endl;
    
    // Create a simple 20x20 grid
    const float gridSize = 20.0f;
    const int gridDivisions = 20;
    const float step = gridSize / gridDivisions;
    
    const int vertexCount = (gridDivisions + 1) * (gridDivisions + 1);
    std::vector<Vertex> vertices(vertexCount);
    
    int index = 0;
    for (int z = 0; z <= gridDivisions; ++z)
    {
        for (int x = 0; x <= gridDivisions; ++x)
        {
            vertices[index].position = DirectX::XMFLOAT3(
                -gridSize / 2.0f + x * step,
                0.0f,  // Ground plane at Y=0 (visible level)
                -gridSize / 2.0f + z * step
            );
            
            // Ground plane normal pointing UP
            vertices[index].normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            
            // BRIGHT checkerboard pattern for visibility
            float checker = ((x + z) % 2 == 0) ? 1.0f : 0.7f;
            vertices[index].color = DirectX::XMFLOAT4(
                0.8f * checker,  // Bright red
                0.2f * checker,  // Low green
                0.2f * checker,  // Low blue
                1.0f
            );
            index++;
        }
    }
    
    // Create indices
    const int indexCount = gridDivisions * gridDivisions * 6;
    std::vector<UINT> indices(indexCount);
    index = 0;
    
    for (int z = 0; z < gridDivisions; ++z)
    {
        for (int x = 0; x < gridDivisions; ++x)
        {
            UINT topLeft = z * (gridDivisions + 1) + x;
            UINT topRight = topLeft + 1;
            UINT bottomLeft = topLeft + (gridDivisions + 1);
            UINT bottomRight = bottomLeft + 1;
            
            indices[index++] = topLeft;
            indices[index++] = bottomLeft;
            indices[index++] = topRight;
            
            indices[index++] = topRight;
            indices[index++] = bottomLeft;
            indices[index++] = bottomRight;
        }
    }
    
    m_groundIndexCount = indexCount;
    
    // Create vertex buffer
    const UINT64 vertexBufferSize = vertexCount * sizeof(Vertex);
    CD3DX12_HEAP_PROPERTIES vbHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC vbBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    
    ThrowIfFailed(
        m_device->CreateCommittedResource(
            &vbHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &vbBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_groundVertexBuffer)
        ),
        "Create ground vertex buffer failed"
    );
    
    UINT8* pVBDataBegin;
    m_groundVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pVBDataBegin));
    memcpy(pVBDataBegin, vertices.data(), vertexBufferSize);
    m_groundVertexBuffer->Unmap(0, nullptr);
    
    m_groundVertexBufferView.BufferLocation = m_groundVertexBuffer->GetGPUVirtualAddress();
    m_groundVertexBufferView.StrideInBytes = sizeof(Vertex);
    m_groundVertexBufferView.SizeInBytes = vertexBufferSize;
    
    // Create index buffer
    const UINT64 indexBufferSize = indexCount * sizeof(UINT);
    CD3DX12_HEAP_PROPERTIES ibHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC ibBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
    
    ThrowIfFailed(
        m_device->CreateCommittedResource(
            &ibHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &ibBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_groundIndexBuffer)
        ),
        "Create ground index buffer failed"
    );
    
    UINT8* pIBDataBegin;
    m_groundIndexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pIBDataBegin));
    memcpy(pIBDataBegin, indices.data(), indexBufferSize);
    m_groundIndexBuffer->Unmap(0, nullptr);
    
    m_groundIndexBufferView.BufferLocation = m_groundIndexBuffer->GetGPUVirtualAddress();
    m_groundIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_groundIndexBufferView.SizeInBytes = indexBufferSize;
    
    std::cout << "  Ground plane created: " << vertexCount << " vertices, " 
              << indexCount << " indices" << std::endl;
    return true;
}

bool GraphicsEngine::CreateCameraConstantBuffer()
{
    std::cout << "  Creating camera constant buffer..." << std::endl;
    
    // Constant buffer size must be 256-byte aligned
    const UINT bufferSize = (sizeof(CameraConstants) + 255) & ~255;
    
    CD3DX12_HEAP_PROPERTIES cbHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC cbBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    
    ThrowIfFailed(
        m_device->CreateCommittedResource(
            &cbHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &cbBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_cameraConstantBuffer)
        ),
        "Create camera constant buffer failed"
    );
    
    // Map and keep mapped
    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(
        m_cameraConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCameraConstantData)),
        "Map camera constant buffer failed"
    );
    
    // Create CBV
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = m_cameraConstantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = bufferSize;
    
    m_device->CreateConstantBufferView(
        &cbvDesc,
        m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart()
    );
    
    std::cout << "  Camera constant buffer created (" << bufferSize << " bytes)" << std::endl;
    return true;
}

// ============================================================================
// RENDER LOOP METHODS
// ============================================================================

void GraphicsEngine::UpdateCameraConstantBuffer(Engine::Rendering::BaseCameraController* camera)
{
    if (!camera || !m_pCameraConstantData)
        return;
    
    CameraConstants constants = {};
    
    // Get view and projection matrices from camera
    DirectX::XMMATRIX view = camera->GetViewMatrix();
    DirectX::XMMATRIX projection = camera->GetProjectionMatrix();
    
    // Store in constant buffer (use temporary variables for l-value)
    DirectX::XMFLOAT4X4 viewFloat;
    DirectX::XMStoreFloat4x4(&viewFloat, view);
    
    DirectX::XMFLOAT4X4 projectionFloat;
    DirectX::XMStoreFloat4x4(&projectionFloat, projection);
    
    constants.viewMatrix = viewFloat;
    constants.projectionMatrix = projectionFloat;
    
    // Debug: Print FULL view matrix on first few frames
    static int frameCount = 0;
    if (frameCount < 3) {
        std::cout << "[GPU] Frame " << frameCount << " - View Matrix:" << std::endl;
        std::cout << "  [" << viewFloat._11 << ", " << viewFloat._12 << ", " << viewFloat._13 << ", " << viewFloat._14 << "]" << std::endl;
        std::cout << "  [" << viewFloat._21 << ", " << viewFloat._22 << ", " << viewFloat._23 << ", " << viewFloat._24 << "]" << std::endl;
        std::cout << "  [" << viewFloat._31 << ", " << viewFloat._32 << ", " << viewFloat._33 << ", " << viewFloat._34 << "]" << std::endl;
        std::cout << "  [" << viewFloat._41 << ", " << viewFloat._42 << ", " << viewFloat._43 << ", " << viewFloat._44 << "]" << std::endl;
        std::cout << "[GPU] Frame " << frameCount << " - Projection Matrix:" << std::endl;
        std::cout << "  [" << projectionFloat._11 << ", " << projectionFloat._12 << ", " << projectionFloat._13 << ", " << projectionFloat._14 << "]" << std::endl;
        std::cout << "  [" << projectionFloat._21 << ", " << projectionFloat._22 << ", " << projectionFloat._23 << ", " << projectionFloat._24 << "]" << std::endl;
        std::cout << "  [" << projectionFloat._31 << ", " << projectionFloat._32 << ", " << projectionFloat._33 << ", " << projectionFloat._34 << "]" << std::endl;
        std::cout << "  [" << projectionFloat._41 << ", " << projectionFloat._42 << ", " << projectionFloat._43 << ", " << projectionFloat._44 << "]" << std::endl;
    }
    frameCount++;
    
    // Copy to mapped memory
    memcpy(m_pCameraConstantData, &constants, sizeof(constants));
}

void GraphicsEngine::PopulateCommandList(Engine::Rendering::BaseCameraController* camera, const std::vector<CreatureMeshData>& creatures)
{
    // Reset command allocator and list
    HRESULT hr = m_commandAllocators[m_frameIndex]->Reset();
    if (FAILED(hr))
    {
        std::cerr << "Reset command allocator failed: 0x" << std::hex << hr << std::dec << std::endl;
        return;
    }
    
    hr = m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);
    if (FAILED(hr))
    {
        std::cerr << "Reset command list failed: 0x" << std::hex << hr << std::dec << std::endl;
        return;
    }
    
    // Set resource barriers
    D3D12_RESOURCE_BARRIER rtvBarrier = {};
    rtvBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rtvBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    rtvBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    rtvBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    rtvBarrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    
    // Only one barrier needed - RTV transition (depth buffer stays in DEPTH_WRITE state)
    m_commandList->ResourceBarrier(1, &rtvBarrier);
    
    // Set viewport and scissor
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_width);
    viewport.Height = static_cast<float>(m_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    
    m_commandList->RSSetViewports(1, &viewport);
    
    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = m_width;
    scissorRect.bottom = m_height;
    
    m_commandList->RSSetScissorRects(1, &scissorRect);
    
    // Clear render target and depth buffer
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_frameIndex,
        m_rtvDescriptorSize
    );
    
    // Use TOD-driven clear color (sky horizon color)
    const FLOAT clearColor[] = { 
        m_todConfig.skyHorizonColor.x,
        m_todConfig.skyHorizonColor.y,
        m_todConfig.skyHorizonColor.z,
        1.0f 
    };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    
    // Clear depth buffer
    m_commandList->ClearDepthStencilView(
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH,
        1.0f,
        0,
        0,
        nullptr
    );
    
    // DO NOT bind render targets yet - shadow map pass will change them
    // We'll bind them after the shadow pass completes
    
    // Set pipeline state
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    
    // Set pipeline state based on wireframe mode (for ALL objects)
    if (m_wireframeMode && m_wireframePipelineState)
    {
        m_commandList->SetPipelineState(m_wireframePipelineState.Get());
    }
    else
    {
        m_commandList->SetPipelineState(m_pipelineState.Get());
    }
    
    // Update and set camera constant buffer
    UpdateCameraConstantBuffer(camera);
    
    // Bind camera CBV directly using root parameter
    m_commandList->SetGraphicsRootConstantBufferView(
        0,
        m_cameraConstantBuffer->GetGPUVirtualAddress()
    );
    
    // Set primitive topology
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // SHADOW MAP PASS: DISABLED - causes DXGI_ERROR_DEVICE_REMOVED (GPU crash)
    // The shadow map implementation has a fundamental issue causing GPU device removal
    // TODO: Debug shadow map PSO and resource barriers
    // UpdateShadowMapConstantBuffer();
    // RenderShadowMap();
    
    // CRITICAL: Reset ALL state after shadow pass
    // The shadow pass leaves the GPU in a different state that breaks main scene rendering
    
    // Reset viewport, scissor, and render targets for main scene
    D3D12_VIEWPORT mainViewport = { 0.0f, 0.0f, (float)m_width, (float)m_height, 0.0f, 1.0f };
    m_commandList->RSSetViewports(1, &mainViewport);
    
    D3D12_RECT mainScissorRect = { 0, 0, (LONG)m_width, (LONG)m_height };
    m_commandList->RSSetScissorRects(1, &mainScissorRect);
    
    // NOW bind render targets for main scene (after shadow pass)
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    
    // Reset PSO, root signature, and primitive topology to main scene state
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    if (m_wireframeMode && m_wireframePipelineState)
    {
        m_commandList->SetPipelineState(m_wireframePipelineState.Get());
    }
    else
    {
        m_commandList->SetPipelineState(m_pipelineState.Get());
    }
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // Render sky dome FIRST (background - doesn't write depth)
    RenderSkyDome();
    
    // Reset to basic PSO and root signature for ground plane and other geometry
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    if (m_wireframeMode && m_wireframePipelineState)
    {
        m_commandList->SetPipelineState(m_wireframePipelineState.Get());
    }
    else
    {
        m_commandList->SetPipelineState(m_pipelineState.Get());
    }
    
    // Draw ground plane (reference grid)
    m_commandList->IASetVertexBuffers(0, 1, &m_groundVertexBufferView);
    m_commandList->IASetIndexBuffer(&m_groundIndexBufferView);
    m_commandList->DrawIndexedInstanced(m_groundIndexCount, 1, 0, 0, 0);
    
    // Draw 3D pyramid as reference object
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(12, 1, 0, 0);
    
    // Render creature meshes
    static bool printed = false;
    if (!creatures.empty())
    {
        if (!printed) {
            std::cout << "[DEBUG] Rendering " << creatures.size() << " creatures" << std::endl;
            printed = true;
        }
        RenderCreatures(creatures, camera);
    } else {
        if (!printed) {
            std::cout << "[DEBUG] NO CREATURES TO RENDER!" << std::endl;
            printed = true;
        }
    }
    
    // Render ImGui UI (BEFORE closing command list)
    if (m_imguiRenderer)
    {
        m_imguiRenderer->RenderDrawData(m_commandList.Get());
    }
    
    // Close command list
    HRESULT hrClose = m_commandList->Close();
    if (FAILED(hrClose))
    {
        std::cerr << "Close command list failed (HRESULT: 0x" << std::hex << hrClose << std::dec << ")" << std::endl;
        return;
    }
}

void GraphicsEngine::RenderCreatures(const std::vector<CreatureMeshData>& creatures, Engine::Rendering::BaseCameraController* camera)
{
    if (!m_commandList)
    {
        std::cerr << "RenderCreatures: Invalid command list!" << std::endl;
        return;
    }
    
    static int frameCounter = 0;
    if (frameCounter < 3) {
        std::cout << "\n=== RenderCreatures Frame " << frameCounter << " ==="  << std::endl;
        std::cout << "Rendering " << creatures.size() << " creatures" << std::endl;
        std::cout << "[Using PBR pipeline with lighting]" << std::endl;
    }
    
    // Use PBR pipeline with lighting
    m_commandList->SetGraphicsRootSignature(m_pbrRootSignature.Get());
    m_commandList->SetPipelineState(
        m_wireframeMode ? m_pbrWireframePipelineState.Get() : m_pbrPipelineState.Get()
    );
    
    // CRITICAL: Bind view/projection matrix CBV (root parameter 0)
    // This was causing creatures to be invisible - vertex shader was reading garbage GPU memory
    m_commandList->SetGraphicsRootConstantBufferView(
        0,
        m_cameraConstantBuffer->GetGPUVirtualAddress()
    );
    
    // Set default material constant buffer (b1) - creatures use vertex color for now
    struct DefaultMaterialCB {
        DirectX::XMFLOAT3 albedo;
        float padding1;
        float roughness;
        float metallic;
        float ambientOcclusion;
        float emissiveIntensity;
        DirectX::XMFLOAT3 emissive;
        float padding2;
    } defaultMaterial;
    
    defaultMaterial.albedo = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f); // White (use vertex color)
    defaultMaterial.roughness = 0.6f;
    defaultMaterial.metallic = 0.0f;
    defaultMaterial.ambientOcclusion = 1.0f;
    defaultMaterial.emissiveIntensity = 0.0f;
    defaultMaterial.emissive = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    defaultMaterial.padding1 = 0.0f;
    defaultMaterial.padding2 = 0.0f;
    
    // Create/upload material CB (static, created once)
    static Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer;
    static UINT8* materialData = nullptr;
    
    if (!materialBuffer)
    {
        D3D12_HEAP_PROPERTIES uploadHeapProps = {};
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = sizeof(DefaultMaterialCB);
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        
        HRESULT hr = m_device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&materialBuffer)
        );
        
        if (SUCCEEDED(hr))
        {
            materialBuffer->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
        }
    }
    
    if (materialData)
    {
        memcpy(materialData, &defaultMaterial, sizeof(DefaultMaterialCB));
        m_commandList->SetGraphicsRootConstantBufferView(1, materialBuffer->GetGPUVirtualAddress());
    }
    
    // Set light constant buffer (b2)
    m_commandList->SetGraphicsRootConstantBufferView(2, m_lightConstantBuffer->GetGPUVirtualAddress());
    
    // Set camera position constant buffer (b3)
    struct CameraPosCB {
        DirectX::XMFLOAT3 position;
        float pad;
    } cameraPosCB;
    // TODO: Add GetPosition() to BaseCameraController interface
    // For now, use a default position (camera position is not critical for basic PBR)
    cameraPosCB.position = DirectX::XMFLOAT3(0, 5, 10);
    cameraPosCB.pad = 0.0f;
    
    // Create temporary upload buffer for camera position
    static Microsoft::WRL::ComPtr<ID3D12Resource> cameraPosBuffer;
    static UINT8* cameraPosData = nullptr;
    
    if (!cameraPosBuffer)
    {
        D3D12_HEAP_PROPERTIES uploadHeapProps = {};
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = sizeof(CameraPosCB);
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        
        HRESULT hr = m_device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&cameraPosBuffer)
        );
        
        if (SUCCEEDED(hr))
        {
            cameraPosBuffer->Map(0, nullptr, reinterpret_cast<void**>(&cameraPosData));
        }
    }
    
    if (cameraPosData)
    {
        memcpy(cameraPosData, &cameraPosCB, sizeof(CameraPosCB));
        m_commandList->SetGraphicsRootConstantBufferView(3, cameraPosBuffer->GetGPUVirtualAddress());
    }
    
    // Bind shadow map constant buffer (b4)
    m_commandList->SetGraphicsRootConstantBufferView(4, m_shadowMapConstantBuffer->GetGPUVirtualAddress());
    
    // Bind shadow map SRV (t0) - root parameter 5 is descriptor table
    ID3D12DescriptorHeap* pShadowHeap = m_shadowMapSrvHeap.Get();
    m_commandList->SetDescriptorHeaps(1, &pShadowHeap);
    m_commandList->SetGraphicsRootDescriptorTable(
        5,
        m_shadowMapSrvHeap->GetGPUDescriptorHandleForHeapStart()
    );
    
    // Render each creature mesh
    for (size_t i = 0; i < creatures.size(); ++i)
    {
        const auto& creature = creatures[i];
        
        if (!creature.meshRenderer)
        {
            std::cerr << "RenderCreatures: Creature " << i << " (" << creature.creatureID << ") has no mesh renderer!" << std::endl;
            continue;
        }
        
        UINT vertexCount = creature.meshRenderer->GetVertexCount();
        if (frameCounter < 3) {
            std::cout << "Rendering creature " << i << " (" << creature.creatureID << ") with " << vertexCount << " vertices at position (" 
                      << creature.position.x << ", " << creature.position.y << ", " << creature.position.z << ")" << std::endl;
        }
        
        if (vertexCount == 0)
        {
            std::cerr << "RenderCreatures: Creature " << i << " has no vertices!" << std::endl;
            continue;
        }
        
        // Render the creature mesh using PBR pipeline
        creature.meshRenderer->Render(m_commandList.Get());
    }
    
    frameCounter++;
}

void GraphicsEngine::WaitForPreviousFrame()
{
    // Wait for the GPU to finish with the command allocator for this frame
    const UINT64 fenceToWaitFor = m_fenceValues[m_frameIndex];
    
    if (m_fence->GetCompletedValue() < fenceToWaitFor)
    {
        ThrowIfFailed(
            m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent),
            "SetEventOnCompletion failed"
        );
        
        // Wait for GPU to complete (with timeout to detect issues)
        DWORD result = WaitForSingleObject(m_fenceEvent, 1000);
        if (result == WAIT_TIMEOUT)
        {
            std::cerr << "GPU fence timeout waiting for value " << fenceToWaitFor 
                     << " (completed: " << m_fence->GetCompletedValue() << ")" << std::endl;
        }
    }
    
    // Update frame index to the current back buffer
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void GraphicsEngine::MoveToNextFrame()
{
    // Assign a new fence value to this frame and signal it
    const UINT64 newFenceValue = m_fenceValues[m_frameIndex] + 1;
    
    ThrowIfFailed(
        m_commandQueue->Signal(m_fence.Get(), newFenceValue),
        "Signal fence (MoveToNextFrame) failed"
    );
    
    // Update fence value for next time we use this frame
    m_fenceValues[m_frameIndex] = newFenceValue;
}

// ============================================================================
// PBR System Implementation
// ============================================================================

bool GraphicsEngine::InitializePBRSystem()
{
    std::cout << "  Initializing PBR system..." << std::endl;
    
    // Step 1: Compile PBR shaders
    if (!CompilePBRShaders())
    {
        std::cerr << "  Failed to compile PBR shaders" << std::endl;
        return false;
    }
    
    // Step 2: Create PBR root signature
    if (!CreatePBRRootSignature())
    {
        std::cerr << "  Failed to create PBR root signature" << std::endl;
        return false;
    }
    
    // Step 3: Create PBR pipeline state
    if (!CreatePBRPipelineState())
    {
        std::cerr << "  Failed to create PBR pipeline state" << std::endl;
        return false;
    }
    
    // Step 3.5: Create PBR wireframe pipeline state
    if (!CreatePBRWireframePipelineState())
    {
        std::cerr << "  Failed to create PBR wireframe pipeline state" << std::endl;
        return false;
    }
    
    // Step 4: Initialize material system
    m_materialSystem = std::make_unique<MaterialSystem>();
    if (!m_materialSystem->Initialize(m_device.Get()))
    {
        std::cerr << "  Failed to initialize material system" << std::endl;
        return false;
    }
    
    // Step 5: Initialize HDR renderer
    m_hdrRenderer = std::make_unique<HDRRenderer>();
    if (!m_hdrRenderer->Initialize(m_device.Get(), m_width, m_height, 
                                    m_commandQueue.Get(), m_factory.Get()))
    {
        std::cerr << "  Failed to initialize HDR renderer" << std::endl;
        return false;
    }
    
    std::cout << "  PBR system initialized successfully" << std::endl;
    
    // Step 5: Initialize lighting and sky dome
    if (!CreateLightConstantBuffer())
    {
        std::cerr << "  Failed to create light constant buffer" << std::endl;
        return false;
    }
    
    if (!CreateSkyDome())
    {
        std::cerr << "  Failed to create sky dome" << std::endl;
        return false;
    }
    
    if (!CreateSkyDomePipelineState())
    {
        std::cerr << "  Failed to create sky dome PSO" << std::endl;
        return false;
    }
    
    // Create shadow map PSO
    if (!CreateShadowMapPipelineState())
    {
        std::cerr << "  WARNING: Failed to create shadow map PSO - shadows will be disabled" << std::endl;
        // Don't return false - allow engine to continue without shadows
    }
    
    // Step 6: Initialize UI button
    InitializeUIButton();
    
    return true;
}

bool GraphicsEngine::CompilePBRShaders()
{
    std::cout << "  Compiling PBR shaders..." << std::endl;
    
    // Create include handler
    ShaderIncludeHandler includeHandler;
    
    // Compile PBR vertex shader
    Microsoft::WRL::ComPtr<ID3DBlob> vertexError;
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    
    HRESULT hr = D3DCompileFromFile(
        L"PBRVertex.hlsl",
        nullptr,
        &includeHandler,
        "main",
        "vs_5_1",
        compileFlags,
        0,
        &m_pbrVertexShaderBlob,
        &vertexError
    );
    
    if (FAILED(hr))
    {
        if (vertexError)
        {
            std::cerr << "PBR Vertex shader compilation error: " 
                      << (char*)vertexError->GetBufferPointer() << std::endl;
        }
        return false;
    }
    
    std::cout << "  PBR Vertex shader compiled successfully" << std::endl;
    
    // Compile PBR pixel shader
    Microsoft::WRL::ComPtr<ID3DBlob> pixelError;
    hr = D3DCompileFromFile(
        L"PBRPixel.hlsl",
        nullptr,
        &includeHandler,
        "main",
        "ps_5_1",
        compileFlags,
        0,
        &m_pbrPixelShaderBlob,
        &pixelError
    );
    
    if (FAILED(hr))
    {
        if (pixelError)
        {
            std::cerr << "PBR Pixel shader compilation error: " 
                      << (char*)pixelError->GetBufferPointer() << std::endl;
        }
        return false;
    }
    
    std::cout << "  PBR Pixel shader compiled successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreatePBRRootSignature()
{
    std::cout << "  Creating PBR root signature..." << std::endl;
    
    // Create root signature with multiple CBVs for PBR:
    // b0: View/Projection matrices (VS)
    // b1: Material constants (PS)
    // b2: Light constants (PS)
    // b3: Camera position (PS)
    // b4: Shadow map matrices (PS)
    // t0: Shadow map texture (PS) - descriptor table
    // s0: Shadow sampler (PS) - static sampler
    
    D3D12_ROOT_PARAMETER rootParams[6] = {};
    
    // b0: View/Projection (Vertex Shader)
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    
    // b1: Material (Pixel Shader)
    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister = 1;  // Matches shader's register(b1)
    rootParams[1].Descriptor.RegisterSpace = 0;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // b2: Lights (Pixel Shader)
    rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[2].Descriptor.ShaderRegister = 2;  // Matches shader's register(b2)
    rootParams[2].Descriptor.RegisterSpace = 0;
    rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // b3: Camera (Pixel Shader)
    rootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[3].Descriptor.ShaderRegister = 3;  // Matches shader's register(b3)
    rootParams[3].Descriptor.RegisterSpace = 0;
    rootParams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // b4: Shadow matrices (Pixel Shader)
    rootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[4].Descriptor.ShaderRegister = 4;  // Matches shader's register(b4)
    rootParams[4].Descriptor.RegisterSpace = 0;
    rootParams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // t0: Shadow map SRV (Pixel Shader) - descriptor table
    rootParams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    
    // CRITICAL: Descriptor range must persist for lifetime of root signature
    // Use static storage to avoid dangling pointer
    static D3D12_DESCRIPTOR_RANGE srvRange = {};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = 1;
    srvRange.BaseShaderRegister = 0;
    srvRange.RegisterSpace = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    
    rootParams[5].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[5].DescriptorTable.pDescriptorRanges = &srvRange;
    rootParams[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // Static sampler for shadow map (comparison sampler for PCF)
    D3D12_STATIC_SAMPLER_DESC shadowSampler = {};
    shadowSampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    shadowSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    shadowSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    shadowSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    shadowSampler.MipLODBias = 0.0f;
    shadowSampler.MaxAnisotropy = 0;
    shadowSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    shadowSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    shadowSampler.MinLOD = 0.0f;
    shadowSampler.MaxLOD = D3D12_FLOAT32_MAX;
    shadowSampler.ShaderRegister = 0;
    shadowSampler.RegisterSpace = 0;
    shadowSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.NumParameters = 6;  // b0-b4 CBVs + 1 descriptor table
    rootDesc.pParameters = rootParams;
    rootDesc.NumStaticSamplers = 1;
    rootDesc.pStaticSamplers = &shadowSampler;
    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    
    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    
    HRESULT hr = D3D12SerializeRootSignature(
        &rootDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error
    );
    
    if (FAILED(hr))
    {
        if (error)
        {
            std::cerr << "PBR Root signature serialization error: " 
                      << (char*)error->GetBufferPointer() << std::endl;
        }
        return false;
    }
    
    ThrowIfFailed(
        m_device->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&m_pbrRootSignature)
        ),
        "Create PBR RootSignature failed"
    );
    
    std::cout << "  PBR Root signature created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreatePBRPipelineState()
{
    std::cout << "  Creating PBR pipeline state object..." << std::endl;
    
    // Define input layout for PBR (position, normal, color)
    // NOTE: Creature meshes use COLOR (float4), not TEXCOORD (float2)
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    
    // Describe and create graphics pipeline state object (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = m_pbrRootSignature.Get();
    psoDesc.VS = { 
        reinterpret_cast<UINT8*>(m_pbrVertexShaderBlob->GetBufferPointer()), 
        m_pbrVertexShaderBlob->GetBufferSize() 
    };
    psoDesc.PS = { 
        reinterpret_cast<UINT8*>(m_pbrPixelShaderBlob->GetBufferPointer()), 
        m_pbrPixelShaderBlob->GetBufferSize() 
    };
    
    // Rasterizer state - TEMPORARILY disable culling for debugging
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // Disable culling
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    // Depth/stencil state - ENABLED for proper depth testing
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    
    // Blend state
    D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
    rtBlendDesc.BlendEnable = FALSE;
    rtBlendDesc.LogicOpEnable = FALSE;
    rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    
    psoDesc.BlendState.RenderTarget[0] = rtBlendDesc;
    
    // Other state
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc = { 1, 0 };
    
    // Enable D3D12 debug layer for detailed validation messages
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(m_device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
    {
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
    }
    
    ThrowIfFailed(
        m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pbrPipelineState)),
        "Create PBR PipelineState failed"
    );
    
    std::cout << "  PBR Pipeline state created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreatePBRWireframePipelineState()
{
    std::cout << "  Creating PBR wireframe pipeline state object..." << std::endl;
    
    // Define input layout for PBR (position, normal, color)
    // NOTE: Creature meshes use COLOR (float4), not TEXCOORD (float2)
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    
    // Describe and create graphics pipeline state object (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = m_pbrRootSignature.Get();
    psoDesc.VS = { 
        reinterpret_cast<UINT8*>(m_pbrVertexShaderBlob->GetBufferPointer()), 
        m_pbrVertexShaderBlob->GetBufferSize() 
    };
    psoDesc.PS = { 
        reinterpret_cast<UINT8*>(m_pbrPixelShaderBlob->GetBufferPointer()), 
        m_pbrPixelShaderBlob->GetBufferSize() 
    };
    
    // Rasterizer state - WIREFRAME MODE
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    // Depth/stencil state - ENABLED for proper 3D rendering
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    
    // Blend state
    D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
    rtBlendDesc.BlendEnable = FALSE;
    rtBlendDesc.LogicOpEnable = FALSE;
    rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    
    psoDesc.BlendState.RenderTarget[0] = rtBlendDesc;
    
    // Other state
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc = { 1, 0 };
    
    ThrowIfFailed(
        m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pbrWireframePipelineState)),
        "Create PBR Wireframe PipelineState failed"
    );
    
    std::cout << "  PBR Wireframe Pipeline state created successfully" << std::endl;
    return true;
}

// ============================================================================
// UI Button Implementation
// ============================================================================

void GraphicsEngine::InitializeUIButton()
{
    // Position button in top-left corner
    m_wireframeButton.x = 10.0f;
    m_wireframeButton.y = 10.0f;
    m_wireframeButton.width = 150.0f;
    m_wireframeButton.height = 40.0f;
    m_wireframeButton.text = L"Wireframe: OFF";
    m_wireframeButton.hovered = false;
    m_wireframeButton.pressed = false;
    
    std::cout << "  [UI] Wireframe button initialized at (" 
              << m_wireframeButton.x << ", " << m_wireframeButton.y << ")" << std::endl;
}

void GraphicsEngine::CheckButtonHover()
{
    if (m_mouseInWindow)
    {
        m_wireframeButton.hovered = IsPointInButton(m_mouseX, m_mouseY);
    }
    else
    {
        m_wireframeButton.hovered = false;
    }
}

bool GraphicsEngine::IsPointInButton(int x, int y) const
{
    return (x >= m_wireframeButton.x && 
            x <= m_wireframeButton.x + m_wireframeButton.width &&
            y >= m_wireframeButton.y && 
            y <= m_wireframeButton.y + m_wireframeButton.height);
}

void GraphicsEngine::OnMouseMove(int x, int y)
{
    m_mouseInWindow = true;
    m_mouseX = x;
    m_mouseY = y;
    CheckButtonHover();
}

void GraphicsEngine::OnMouseLeave()
{
    m_mouseInWindow = false;
    m_wireframeButton.hovered = false;
}

void GraphicsEngine::OnMouseClick(int x, int y)
{
    if (IsPointInButton(x, y))
    {
        ToggleWireframe();
        m_wireframeButton.pressed = true;
        
        // Update button text
        m_wireframeButton.text = m_wireframeMode ? L"Wireframe: ON" : L"Wireframe: OFF";
        
        std::cout << "[Wireframe] Toggled to: " << (m_wireframeMode ? "ON" : "OFF") << std::endl;
    }
}

// ============================================================================
// Lighting and Sky Dome Implementation
// ============================================================================

bool GraphicsEngine::CreateLightConstantBuffer()
{
    std::cout << "  Creating light constant buffer..." << std::endl;
    
    // Initialize TOD config with default values
    m_todConfig.sunAngle = 0.0f;
    m_todConfig.sunColor = { 1.0f, 0.95f, 0.8f };
    m_todConfig.skyZenithColor = { 0.3f, 0.5f, 0.9f };  // Blue sky
    m_todConfig.skyHorizonColor = { 0.6f, 0.75f, 0.95f }; // Lighter blue at horizon
    m_todConfig.groundColor = { 0.2f, 0.25f, 0.15f };   // Dark green-brown ground
    m_todConfig.ambientIntensity = 0.3f;
    
    // Create upload heap for light constant buffer
    D3D12_HEAP_PROPERTIES uploadHeapProps = {};
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    uploadHeapProps.CreationNodeMask = 1;
    uploadHeapProps.VisibleNodeMask = 1;
    
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = sizeof(LightConstants);
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
    HRESULT hr = m_device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_lightConstantBuffer)
    );
    
    if (FAILED(hr))
    {
        std::cerr << "Failed to create light constant buffer" << std::endl;
        return false;
    }
    
    // Map the buffer
    hr = m_lightConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_lightCBVData));
    if (FAILED(hr))
    {
        std::cerr << "Failed to map light constant buffer" << std::endl;
        return false;
    }
    
    // Initialize with default lighting
    UpdateLightingFromTOD();
    
    std::cout << "  Light constant buffer created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateSkyDome()
{
    std::cout << "  Creating sky dome geometry..." << std::endl;
    
    // Create a sphere mesh (radius = 500.0f)
    const float radius = 500.0f;
    const int slices = 32;
    const int stacks = 16;
    
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<uint32_t> indices;
    
    // Generate sphere vertices
    for (int stack = 0; stack <= stacks; stack++)
    {
        float phi = DirectX::XM_PI * stack / stacks; // 0 to PI
        float y = radius * cosf(phi);
        float ringRadius = radius * sinf(phi);
        
        for (int slice = 0; slice <= slices; slice++)
        {
            float theta = 2.0f * DirectX::XM_PI * slice / slices; // 0 to 2*PI
            float x = ringRadius * cosf(theta);
            float z = ringRadius * sinf(theta);
            
            vertices.push_back({ x, y, z });
        }
    }
    
    // Generate indices
    for (int stack = 0; stack < stacks; stack++)
    {
        for (int slice = 0; slice < slices; slice++)
        {
            uint32_t current = stack * (slices + 1) + slice;
            uint32_t next = current + slices + 1;
            
            // Two triangles per quad
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);
            
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
    
    m_skyDomeVertexCount = static_cast<UINT>(vertices.size());
    m_skyDomeIndexCount = static_cast<UINT>(indices.size());
    
    // Create upload buffer for vertices
    D3D12_HEAP_PROPERTIES uploadHeapProps = {};
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    
    D3D12_RESOURCE_DESC vertexUploadDesc = {};
    vertexUploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexUploadDesc.Width = vertices.size() * sizeof(DirectX::XMFLOAT3);
    vertexUploadDesc.Height = 1;
    vertexUploadDesc.DepthOrArraySize = 1;
    vertexUploadDesc.MipLevels = 1;
    vertexUploadDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexUploadDesc.SampleDesc.Count = 1;
    vertexUploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexUploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
    HRESULT hr = m_device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &vertexUploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_skyDomeVertexBuffer)
    );
    
    if (FAILED(hr))
    {
        std::cerr << "Failed to create sky dome vertex buffer" << std::endl;
        return false;
    }
    
    // Copy vertex data
    void* pVertexData;
    m_skyDomeVertexBuffer->Map(0, nullptr, &pVertexData);
    memcpy(pVertexData, vertices.data(), vertices.size() * sizeof(DirectX::XMFLOAT3));
    m_skyDomeVertexBuffer->Unmap(0, nullptr);
    
    m_skyDomeVertexBufferView.BufferLocation = m_skyDomeVertexBuffer->GetGPUVirtualAddress();
    m_skyDomeVertexBufferView.StrideInBytes = sizeof(DirectX::XMFLOAT3);
    m_skyDomeVertexBufferView.SizeInBytes = static_cast<UINT>(vertexUploadDesc.Width);
    
    // Create upload buffer for indices
    D3D12_RESOURCE_DESC indexUploadDesc = {};
    indexUploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    indexUploadDesc.Width = indices.size() * sizeof(uint32_t);
    indexUploadDesc.Height = 1;
    indexUploadDesc.DepthOrArraySize = 1;
    indexUploadDesc.MipLevels = 1;
    indexUploadDesc.Format = DXGI_FORMAT_UNKNOWN;
    indexUploadDesc.SampleDesc.Count = 1;
    indexUploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    indexUploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
    hr = m_device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &indexUploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_skyDomeIndexBuffer)
    );
    
    if (FAILED(hr))
    {
        std::cerr << "Failed to create sky dome index buffer" << std::endl;
        return false;
    }
    
    // Copy index data
    void* pIndexData;
    m_skyDomeIndexBuffer->Map(0, nullptr, &pIndexData);
    memcpy(pIndexData, indices.data(), indices.size() * sizeof(uint32_t));
    m_skyDomeIndexBuffer->Unmap(0, nullptr);
    
    m_skyDomeIndexBufferView.BufferLocation = m_skyDomeIndexBuffer->GetGPUVirtualAddress();
    m_skyDomeIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_skyDomeIndexBufferView.SizeInBytes = static_cast<UINT>(indexUploadDesc.Width);
    
    std::cout << "  Sky dome created successfully (" << m_skyDomeVertexCount 
              << " vertices, " << m_skyDomeIndexCount << " indices)" << std::endl;
    return true;
}

bool GraphicsEngine::CreateSkyDomePipelineState()
{
    std::cout << "  Creating sky dome pipeline state..." << std::endl;
    
    // Compile shaders
    ShaderIncludeHandler includeHandler;
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    ID3DBlob* errorBlob = nullptr;
    
    HRESULT hr = D3DCompileFromFile(
        L"SkyDomeVertex.hlsl",
        nullptr,
        &includeHandler,
        "main",
        "vs_5_1",
        compileFlags,
        0,
        &m_skyDomeVertexShaderBlob,
        &errorBlob  // Changed from nullptr to capture errors
    );
    
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::cerr << "Sky dome vertex shader compilation error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        else
        {
            std::cerr << "Failed to compile sky dome vertex shader (file not found or no error details)" << std::endl;
        }
        return false;
    }
    
    hr = D3DCompileFromFile(
        L"SkyDomePixel.hlsl",
        nullptr,
        &includeHandler,
        "main",
        "ps_5_1",
        compileFlags,
        0,
        &m_skyDomePixelShaderBlob,
        &errorBlob
    );
    
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::cerr << "Sky dome pixel shader compilation error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        else
        {
            std::cerr << "Failed to compile sky dome pixel shader (no error details)" << std::endl;
        }
        return false;
    }
    
    // Check for warnings even if compilation succeeded
    if (errorBlob)
    {
        std::cout << "Sky dome pixel shader warnings: " << (char*)errorBlob->GetBufferPointer() << std::endl;
        errorBlob->Release();
    }
    
    // Input layout: position only
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    
    // Create PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = m_pbrRootSignature.Get();
    psoDesc.VS = { m_skyDomeVertexShaderBlob->GetBufferPointer(), m_skyDomeVertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { m_skyDomePixelShaderBlob->GetBufferPointer(), m_skyDomePixelShaderBlob->GetBufferSize() };
    
    // Rasterizer state - render back faces (inside of dome)
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // No culling - we're inside the dome
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    // Depth state - Read only, don't write (creatures must render in front of sky)
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // DON'T write depth
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    
    // Blend state
    D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
    rtBlendDesc.BlendEnable = FALSE;
    rtBlendDesc.LogicOpEnable = FALSE;
    rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    
    psoDesc.BlendState.RenderTarget[0] = rtBlendDesc;
    
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc = { 1, 0 };
    
    hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_skyDomePipelineState));
    if (FAILED(hr))
    {
        std::cerr << "Failed to create sky dome pipeline state" << std::endl;
        return false;
    }
    
    std::cout << "  Sky dome pipeline state created successfully" << std::endl;
    return true;
}

bool GraphicsEngine::CreateShadowMapPipelineState()
{
    std::cout << "  Creating shadow map pipeline state..." << std::endl;
    
    // Compile shadow vertex shader
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    
    HRESULT hr = D3DCompileFromFile(
        L"ShadowVS.hlsl",
        nullptr,
        nullptr,
        "main",
        "vs_5_1",
        compileFlags,
        0,
        &m_shadowVertexShaderBlob,
        &error
    );
    
    if (FAILED(hr))
    {
        if (error)
        {
            std::cerr << "Shadow vertex shader compilation error: " << (char*)error->GetBufferPointer() << std::endl;
            error->Release();
        }
        return false;
    }
    
    // Create root signature for shadow map - reuse basic PSO's root signature
    m_shadowMapRootSignature = m_rootSignature;
    
    // Define input layout - MUST match ShadowVS.hlsl (POSITION, NORMAL, COLOR)
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    
    // Create depth-only PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = m_shadowMapRootSignature.Get();
    psoDesc.VS = { m_shadowVertexShaderBlob->GetBufferPointer(), m_shadowVertexShaderBlob->GetBufferSize() };
    psoDesc.PS = {};  // No pixel shader - depth only
    psoDesc.NodeMask = 0;  // Single GPU
    
    // Rasterizer state
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;  // Standard backface culling
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    // Depth state - write depth for shadow map
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    
    // Blend state - initialize even though we have 0 render targets
    D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
    rtBlendDesc.BlendEnable = FALSE;
    rtBlendDesc.LogicOpEnable = FALSE;
    rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.BlendState.RenderTarget[0] = rtBlendDesc;
    
    // No render targets (depth only)
    psoDesc.NumRenderTargets = 0;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    psoDesc.SampleDesc = { 1, 0 };
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    
    hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_shadowMapPipelineState));
    if (FAILED(hr))
    {
        std::cerr << "Failed to create shadow map pipeline state (HRESULT: 0x" << std::hex << hr << std::dec << ")" << std::endl;
        std::cerr << "  Input elements: " << psoDesc.InputLayout.NumElements << std::endl;
        std::cerr << "  NumRenderTargets: " << psoDesc.NumRenderTargets << std::endl;
        std::cerr << "  DSVFormat: " << psoDesc.DSVFormat << std::endl;
        return false;
    }
    
    std::cout << "  Shadow map pipeline state created successfully" << std::endl;
    return true;
}

void GraphicsEngine::UpdateLightingFromTOD()
{
    // Calculate sun direction from angle
    // At time 0.0 (noon): sun should be overhead (0, 1, 0)
    // At time 0.25 (sunset): sun should be at horizon (-1, 0, 0)
    // At time 0.5 (midnight): sun should be below (0, -1, 0)
    m_todConfig.sunAngle = m_currentTimeOfDay * 2.0f * DirectX::XM_PI - DirectX::XM_PIDIV2;
    
    // Sun direction (circular path in Y-Z plane)
    m_lightData.sunDirection = {
        0.3f, // Slight X offset for visual interest
        cosf(m_todConfig.sunAngle),  // Y: up/down
        sinf(m_todConfig.sunAngle)   // Z: forward/back
    };
    
    // Normalize
    float len = sqrtf(
        m_lightData.sunDirection.x * m_lightData.sunDirection.x +
        m_lightData.sunDirection.y * m_lightData.sunDirection.y +
        m_lightData.sunDirection.z * m_lightData.sunDirection.z
    );
    m_lightData.sunDirection.x /= len;
    m_lightData.sunDirection.y /= len;
    m_lightData.sunDirection.z /= len;
    
    // Set TOD-driven colors
    m_lightData.sunColor = m_todConfig.sunColor;
    m_lightData.sunIntensity = 1.5f;
    
    m_lightData.ambientColor = m_todConfig.skyZenithColor;
    m_lightData.ambientIntensity = m_todConfig.ambientIntensity;
    
    m_lightData.groundAmbientColor = m_todConfig.groundColor;
    m_lightData.groundAmbientIntensity = 0.15f;
    
    // Copy to GPU
    memcpy(m_lightCBVData, &m_lightData, sizeof(LightConstants));
    
    // DEBUG: Print lighting values on first 3 frames
    static int debugFrame = 0;
    if (debugFrame < 3) {
        std::cout << "[LIGHTING DEBUG] Frame " << debugFrame << ":" << std::endl;
        std::cout << "  Sun Direction: (" << m_lightData.sunDirection.x << ", " 
                  << m_lightData.sunDirection.y << ", " << m_lightData.sunDirection.z << ")" << std::endl;
        std::cout << "  Sun Color: (" << m_lightData.sunColor.x << ", " 
                  << m_lightData.sunColor.y << ", " << m_lightData.sunColor.z << ")" << std::endl;
        std::cout << "  Sun Intensity: " << m_lightData.sunIntensity << std::endl;
        std::cout << "  Ambient Color: (" << m_lightData.ambientColor.x << ", " 
                  << m_lightData.ambientColor.y << ", " << m_lightData.ambientColor.z << ")" << std::endl;
        std::cout << "  Ambient Intensity: " << m_lightData.ambientIntensity << std::endl;
        debugFrame++;
    }
}

void GraphicsEngine::RenderSkyDome()
{
    if (!m_commandList || !m_skyDomePipelineState) {
        return;
    }
    
    // Set sky dome PSO
    m_commandList->SetPipelineState(m_skyDomePipelineState.Get());
    m_commandList->SetGraphicsRootSignature(m_pbrRootSignature.Get());
    
    // CRITICAL: Bind view/projection matrix CBV (root parameter 0)
    // SkyDomeVertex.hlsl also requires b0 for view/projection transforms
    m_commandList->SetGraphicsRootConstantBufferView(
        0,
        m_cameraConstantBuffer->GetGPUVirtualAddress()
    );
    
    // CRITICAL: Bind light constant buffer (root parameter 2) for sun disc
    m_commandList->SetGraphicsRootConstantBufferView(
        2,
        m_lightConstantBuffer->GetGPUVirtualAddress()
    );
    
    // Set vertex/index buffers
    m_commandList->IASetVertexBuffers(0, 1, &m_skyDomeVertexBufferView);
    m_commandList->IASetIndexBuffer(&m_skyDomeIndexBufferView);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    
    // Draw sky dome
    m_commandList->DrawIndexedInstanced(m_skyDomeIndexCount, 1, 0, 0, 0);
}

void GraphicsEngine::UpdateShadowMapConstantBuffer()
{
    // Calculate light view matrix (looking from sun direction towards origin)
    DirectX::XMFLOAT3 sunPos = {
        -m_lightData.sunDirection.x * 50.0f,
        -m_lightData.sunDirection.y * 50.0f,
        -m_lightData.sunDirection.z * 50.0f
    };
    DirectX::XMFLOAT3 lookAt = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
    
    DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat3(&sunPos),
        DirectX::XMLoadFloat3(&lookAt),
        DirectX::XMLoadFloat3(&up)
    );
    
    // Orthographic projection for shadow map (covers 50x50 area)
    DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicLH(50.0f, 50.0f, 1.0f, 100.0f);
    
    // Transpose matrices for HLSL (row-major)
    lightView = DirectX::XMMatrixTranspose(lightView);
    lightProj = DirectX::XMMatrixTranspose(lightProj);
    
    // Copy to constant buffer
    DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&m_shadowCBVData[0]), lightView);
    DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&m_shadowCBVData[64]), lightProj);
}

void GraphicsEngine::RenderShadowMap()
{
    if (!m_shadowMapPipelineState || !m_shadowMapTexture) return;
    
    // Transition shadow map from its current state to depth write
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_shadowMapTexture.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = m_shadowMapState;  // Use tracked state
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    m_commandList->ResourceBarrier(1, &barrier);
    m_shadowMapState = D3D12_RESOURCE_STATE_DEPTH_WRITE;  // Update tracked state
    
    // Clear shadow map (use shadow map's own DSV, NOT the main scene's)
    CD3DX12_CPU_DESCRIPTOR_HANDLE shadowDSV(m_shadowMapDsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_commandList->ClearDepthStencilView(
        shadowDSV,
        D3D12_CLEAR_FLAG_DEPTH,
        1.0f,
        0,
        0,
        nullptr
    );
    
    // Set shadow map as depth buffer (use shadow map's own DSV)
    m_commandList->OMSetRenderTargets(0, nullptr, FALSE, &shadowDSV);
    
    // Set viewport for shadow map
    D3D12_VIEWPORT shadowViewport = { 0.0f, 0.0f, (float)ShadowMapSize, (float)ShadowMapSize, 0.0f, 1.0f };
    m_commandList->RSSetViewports(1, &shadowViewport);
    
    // Set shadow map PSO and root signature
    m_commandList->SetPipelineState(m_shadowMapPipelineState.Get());
    m_commandList->SetGraphicsRootSignature(m_shadowMapRootSignature.Get());
    
    // Bind shadow map constant buffer
    m_commandList->SetGraphicsRootConstantBufferView(
        0,
        m_shadowMapConstantBuffer->GetGPUVirtualAddress()
    );
    
    // Draw ground plane into shadow map
    m_commandList->IASetVertexBuffers(0, 1, &m_groundVertexBufferView);
    m_commandList->IASetIndexBuffer(&m_groundIndexBufferView);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->DrawIndexedInstanced(m_groundIndexCount, 1, 0, 0, 0);
    
    // Draw creatures into shadow map (simplified - just position)
    // Note: Would need to iterate through genetics integration and draw creature meshes
    
    // Transition shadow map back to shader resource
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    m_commandList->ResourceBarrier(1, &barrier);
    m_shadowMapState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;  // Update tracked state
}



