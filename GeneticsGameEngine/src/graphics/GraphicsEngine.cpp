#include "GraphicsEngine.h"
#include "../engine/rendering/camera/CameraController.h"
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
    
    try
    {
        // Wait for GPU to finish previous frame
        WaitForPreviousFrame();
        
        // Populate command list
        PopulateCommandList(camera);
        
        // Execute command list
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(1, ppCommandLists);
        
        // Present frame
        HRESULT hr = m_swapChain->Present(0, 0);
        if (FAILED(hr))
        {
            std::cerr << "Present failed with HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
        }
        
        // Move to next frame
        MoveToNextFrame();
        
        if (frameCount < 5)
        {
            std::cout << "Frame " << frameCount << " rendered successfully" << std::endl;
        }
        frameCount++;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in Render: " << e.what() << std::endl;
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
    
    // Enable GPU-based validation
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
    if (SUCCEEDED(debugController.As(&debugController1)))
    {
        debugController1->SetEnableGPUBasedValidation(TRUE);
        std::cout << "  GPU-Based Validation enabled" << std::endl;
    }
    
    // Enable debug layer break on error
    Microsoft::WRL::ComPtr<ID3D12Debug2> debugController2;
    if (SUCCEEDED(debugController.As(&debugController2)))
    {
        debugController2->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        debugController2->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        std::cout << "  Debug break on error enabled" << std::endl;
    }
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
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Match PSO format
    depthDesc.SampleDesc = { 1, 0 };
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
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
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
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
        
    // TEST: Use simple shader without camera transformation
    HRESULT hr = D3DCompileFromFile(
        L"vertex_simple.hlsl",
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
    
    // Create empty root signature for test (no descriptors needed for simple shader)
    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.NumParameters = 0;
    rootDesc.pParameters = nullptr;
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
    
    std::cout << "  Root signature created successfully (empty for test)" << std::endl;
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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, 
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
    
    // Rasterizer state
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // Disabled for visibility
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    // Depth/stencil state - DISABLED for testing
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; // More compatible format
    
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

// ============================================================================
// GEOMETRY METHODS
// ============================================================================

bool GraphicsEngine::CreateVertexBuffer()
{
    std::cout << "  Creating vertex buffer (test triangle)..." << std::endl;
    
    // Define LARGE triangle vertices that cover most of the screen in clip space
    Vertex vertices[] =
    {
        { { 0.0f, 0.8f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },   // Top - Red
        { { 0.8f, -0.8f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },  // Bottom Right - Green
        { { -0.8f, -0.8f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }  // Bottom Left - Blue
    };
    
    const UINT64 bufferSize = sizeof(vertices);
    
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
                0.0f,
                -gridSize / 2.0f + z * step
            );
            
            // Checkerboard pattern
            float checker = ((x + z) % 2 == 0) ? 1.0f : 0.8f;
            vertices[index].color = DirectX::XMFLOAT4(
                0.3f * checker,
                0.6f * checker,
                0.2f * checker,
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
    
    // Copy to mapped memory
    memcpy(m_pCameraConstantData, &constants, sizeof(constants));
}

void GraphicsEngine::PopulateCommandList(Engine::Rendering::BaseCameraController* camera)
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
    
    const FLOAT clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    
    // TEMPORARILY DISABLED: Depth buffer clear to test if it's causing GPU rejection
    // m_commandList->ClearDepthStencilView(
    //     m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
    //     D3D12_CLEAR_FLAG_DEPTH,
    //     1.0f,
    //     0,
    //     0,
    //     nullptr
    // );
    
    // CRITICAL: Bind render targets for drawing (THIS WAS MISSING!)
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    
    // Set pipeline state
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->SetPipelineState(m_pipelineState.Get());
    
    // Update and set camera constant buffer
    UpdateCameraConstantBuffer(camera);
    
    // TEMPORARILY DISABLED: Descriptor heap setting to test basic rendering
    // ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvUavHeap.Get() };
    // m_commandList->SetDescriptorHeaps(1, ppHeaps);
    // m_commandList->SetGraphicsRootDescriptorTable(
    //     0,
    //     m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart()
    // );
    
    // Set primitive topology
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // Draw test triangle
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);
    
    // Draw ground plane
    m_commandList->IASetVertexBuffers(0, 1, &m_groundVertexBufferView);
    m_commandList->IASetIndexBuffer(&m_groundIndexBufferView);
    m_commandList->DrawIndexedInstanced(m_groundIndexCount, 1, 0, 0, 0);
    
    // Close command list
    HRESULT hrClose = m_commandList->Close();
    if (FAILED(hrClose))
    {
        std::cerr << "Close command list failed (HRESULT: 0x" << std::hex << hrClose << std::dec << ")" << std::endl;
        return;
    }
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
