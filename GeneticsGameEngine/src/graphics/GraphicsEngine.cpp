#include "GraphicsEngine.h"
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <windows.h>
#include <iostream>
#include <d3d12sdklayers.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <fstream>
#include <string>

// Helper function to check HRESULT
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::runtime_error("DirectX error");
    }
}

bool GraphicsEngine::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;
    
    // Initialize DirectX 12
    std::cout << "Initializing DirectX 12..." << std::endl;
    if (!InitializeDX12())
    {
        std::cerr << "Failed to initialize DirectX 12!" << std::endl;
        return false;
    }
    std::cout << "DirectX 12 initialized successfully." << std::endl;
    
    // Create swap chain
    std::cout << "Creating swap chain..." << std::endl;
    if (!CreateSwapChain())
    {
        std::cerr << "Failed to create swap chain!" << std::endl;
        return false;
    }
    std::cout << "Swap chain created successfully." << std::endl;
    
    // Create command objects
    std::cout << "Creating command objects..." << std::endl;
    if (!CreateCommandObjects())
    {
        std::cerr << "Failed to create command objects!" << std::endl;
        return false;
    }
    std::cout << "Command objects created successfully." << std::endl;
    
    // Create synchronization objects
    std::cout << "Creating synchronization objects..." << std::endl;
    if (!CreateSyncObjects())
    {
        std::cerr << "Failed to create synchronization objects!" << std::endl;
        return false;
    }
    std::cout << "Synchronization objects created successfully." << std::endl;
    
    return true;
}

bool GraphicsEngine::InitializeDX12()
{
    // Create DXGI factory with debug support
    UINT createFactoryFlags = 0;
#ifdef _DEBUG
    createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&m_factory)));
    
    // Create adapter (GPU)
    ThrowIfFailed(m_factory->EnumAdapters(0, &m_adapter));
    
    // Create device
    ThrowIfFailed(D3D12CreateDevice(
        m_adapter,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)
    ));
    
    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    
    return true;
}

bool GraphicsEngine::CreateSwapChain()
{
    // Use stored window dimensions from Window class
    UINT width = 800;
    UINT height = 600;
    std::cout << "Window size: " << width << "x" << height << std::endl;
    
    // Use legacy CreateSwapChain which is more compatible
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = m_hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    HRESULT hr = m_factory->CreateSwapChain(
        m_commandQueue,
        &swapChainDesc,
        &m_swapChain
    );
    
    if (FAILED(hr))
    {
        std::cerr << "CreateSwapChainForHwnd failed with HRESULT: 0x" << std::hex << hr << std::endl;
        return false;
    }
    std::cout << "Swap chain created successfully." << std::endl;
    
    // Query for IDXGISwapChain3 interface
    if (m_swapChain)
    {
        ThrowIfFailed(m_swapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&m_swapChain3));
    }
    
    // Create render target views
    // Create RTV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
    
    // Get RTV descriptor size
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    
    // Create RTVs for each back buffer
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (int i = 0; i < 2; i++)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
        m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);
        
        // Move to next descriptor
        rtvHandle.ptr += m_rtvDescriptorSize;
    }
    
    return true;
}

bool GraphicsEngine::CreateCommandObjects()
{
    // Create command allocator
    ThrowIfFailed(m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&m_commandAllocator)
    ));
    
    // Create command list
    ThrowIfFailed(m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocator,
        nullptr,
        IID_PPV_ARGS(&m_commandList)
    ));
    
    // Close the command list
    m_commandList->Close();
    
    // Close the command list
    m_commandList->Close();
    
    return true;
}

bool GraphicsEngine::CreateSyncObjects()
{
    // Create fence
    std::cout << "Creating fence..." << std::endl;
    ThrowIfFailed(m_device->CreateFence(
        0,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&m_fence)
    ));
    std::cout << "Fence created successfully." << std::endl;

    // Create fence event
    std::cout << "Creating fence event..." << std::endl;
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        std::cerr << "Failed to create fence event!" << std::endl;
        return false;
    }
    std::cout << "Fence event created successfully." << std::endl;

    // Compile shaders
    std::cout << "Compiling shaders..." << std::endl;
    if (!CompileShaders())
    {
        std::cerr << "Failed to compile shaders!" << std::endl;
        return false;
    }
    std::cout << "Shaders compiled successfully." << std::endl;

    // Create root signature
    std::cout << "Creating root signature..." << std::endl;
    if (!CreateRootSignature())
    {
        std::cerr << "Failed to create root signature!" << std::endl;
        return false;
    }
    std::cout << "Root signature created successfully." << std::endl;

    // Create pipeline state object
    std::cout << "Creating pipeline state object..." << std::endl;
    if (!CreatePipelineState())
    {
        std::cerr << "Failed to create pipeline state object!" << std::endl;
        return false;
    }
    std::cout << "Pipeline state object created successfully." << std::endl;

    // Create vertex buffer
    std::cout << "Creating vertex buffer..." << std::endl;
    if (!CreateVertexBuffer())
    {
        std::cerr << "Failed to create vertex buffer!" << std::endl;
        return false;
    }
    std::cout << "Vertex buffer created successfully." << std::endl;

    return true;
}

void GraphicsEngine::Update()
{
    // Update logic here
    std::cout << "Updating graphics engine..." << std::endl;
}

void GraphicsEngine::Render()
{
    // Wait for previous frame
    WaitForPreviousFrame();
    
    // Debug output
    std::cout << "Rendering frame..." << std::endl;
    
    // Reset command allocator
    m_commandAllocator->Reset();
    
    // Reset command list
    m_commandList->Reset(m_commandAllocator, nullptr);
    
    // Set render target
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_commandList->ResourceBarrier(1, &barrier);
    
    // Clear render target
    float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    
    // Get RTV handle
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_rtvDescriptorSize * m_frameIndex;
    
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    
    // Set viewport and scissor rect
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_commandList->RSSetViewports(1, &viewport);
    
    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = 800;
    scissorRect.bottom = 600;
    m_commandList->RSSetScissorRects(1, &scissorRect);
    
    // Set pipeline state and root signature
    m_commandList->SetPipelineState(m_pipelineState);
    m_commandList->SetGraphicsRootSignature(m_rootSignature);
    
    // Set vertex buffer
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    
    // Draw triangle
    m_commandList->DrawInstanced(3, 1, 0, 0);
    
    // Transition back to present state
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_commandList->ResourceBarrier(1, &barrier);
    
    // Close command list
    m_commandList->Close();
    
    // Execute command list
    ID3D12CommandList* ppCommandLists[] = { m_commandList };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    
    // Present
    m_swapChain->Present(1, 0);
    
    // Move to next frame
    MoveToNextFrame();
}

void GraphicsEngine::Render(std::unique_ptr<GeneticsIntegration>& geneticsIntegration, Engine::Rendering::BaseCameraController* camera)
{
    // Wait for previous frame
    WaitForPreviousFrame();
    
    // Debug output
    std::cout << "Rendering frame..." << std::endl;
    
    // Reset command allocator
    m_commandAllocator->Reset();
    
    // Reset command list
    m_commandList->Reset(m_commandAllocator, nullptr);
    
    // Set render target
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_commandList->ResourceBarrier(1, &barrier);
    
    // Clear render target
    float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    
    // Get RTV handle
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_rtvDescriptorSize * m_frameIndex;
    
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    
    // Set viewport and scissor rect
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_commandList->RSSetViewports(1, &viewport);
    
    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = 800;
    scissorRect.bottom = 600;
    m_commandList->RSSetScissorRects(1, &scissorRect);
    
    // Set pipeline state and root signature
    m_commandList->SetPipelineState(m_pipelineState);
    m_commandList->SetGraphicsRootSignature(m_rootSignature);
    
    // Set vertex buffer
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    
    // Draw triangle
    m_commandList->DrawInstanced(3, 1, 0, 0);
    
    // Transition back to present state
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_commandList->ResourceBarrier(1, &barrier);
    
    // Close command list
    m_commandList->Close();
    
    // Execute command list
    ID3D12CommandList* ppCommandLists[] = { m_commandList };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    
    // Present
    m_swapChain->Present(1, 0);
    
    // Move to next frame
    MoveToNextFrame();
}

void GraphicsEngine::WaitForPreviousFrame()
{
    // Signal and wait for fence
    m_commandQueue->Signal(m_fence, m_fenceValue);
    
    if (m_fence->GetCompletedValue() < m_fenceValue)
    {
        m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
        // Use timeout to prevent infinite hang
        DWORD result = WaitForSingleObject(m_fenceEvent, 5000); // 5 second timeout
        if (result == WAIT_TIMEOUT)
        {
            std::cerr << "Warning: WaitForSingleObject timed out after 5 seconds!" << std::endl;
            // Force increment fence value to prevent deadlock
            m_fenceValue++;
            return;
        }
    }
    
    m_fenceValue++;
}

void GraphicsEngine::MoveToNextFrame()
{
    if (m_swapChain3)
        m_frameIndex = m_swapChain3->GetCurrentBackBufferIndex();
}

bool GraphicsEngine::CreateRootSignature()
{
    // Create root signature
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Create root signature descriptor
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    D3D12_ROOT_PARAMETER rootParameter = {};
    D3D12_DESCRIPTOR_RANGE descriptorRange = {};

    descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descriptorRange.NumDescriptors = 1;
    descriptorRange.BaseShaderRegister = 0;
    descriptorRange.RegisterSpace = 0;
    descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.DescriptorTable.NumDescriptorRanges = 1;
    rootParameter.DescriptorTable.pDescriptorRanges = &descriptorRange;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParameter;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* signature = nullptr;
    ID3DBlob* error = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    if (FAILED(hr))
    {
        if (error)
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
            error->Release();
        }
        return false;
    }

    ThrowIfFailed(m_device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)));

    if (signature)
        signature->Release();
    if (error)
        error->Release();

    return true;
}

bool GraphicsEngine::CreatePipelineState()
{
    // Define input layout to match vertex structure
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Create pipeline state object
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = m_rootSignature;
    psoDesc.VS = {m_vertexShaderBlob->GetBufferPointer(), m_vertexShaderBlob->GetBufferSize()};
    psoDesc.PS = {m_pixelShaderBlob->GetBufferPointer(), m_pixelShaderBlob->GetBufferSize()};
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.RasterizerState.MultisampleEnable = FALSE;
    psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    {
        psoDesc.BlendState.RenderTarget[i].BlendEnable = FALSE;
        psoDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
        psoDesc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
        psoDesc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
        psoDesc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
        psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
        psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
        psoDesc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        psoDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
        psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }
    
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DepthStencilState.StencilReadMask = 0xFF;
    psoDesc.DepthStencilState.StencilWriteMask = 0xFF;
    psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    psoDesc.DepthStencilState.BackFace = psoDesc.DepthStencilState.FrontFace;
    
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

    HRESULT hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
    if (FAILED(hr))
    {
        std::cerr << "Failed to create pipeline state with HRESULT: 0x" << std::hex << hr << std::endl;
        return false;
    }

    std::cout << "Pipeline state created successfully." << std::endl;
    return true;
}

bool GraphicsEngine::CreateVertexBuffer()
{
    // Define triangle vertices
    struct Vertex
    {
        float position[3];
        float color[4];
    };

    Vertex vertices[] =
    {
        { { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    const UINT vertexBufferSize = sizeof(vertices);

    // Create vertex buffer resource
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 0;
    heapProps.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = vertexBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
    ThrowIfFailed(m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)));

    // Copy vertex data to the vertex buffer
    UINT8* pVertexDataBegin;
    D3D12_RANGE readRange = {};
    readRange.Begin = 0;
    readRange.End = 0;
    ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, vertices, sizeof(vertices));
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(vertices);
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);

    return true;
}

bool GraphicsEngine::CompileShaders()
{
    // Compile vertex shader
    ID3DBlob* vertexShaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    
    std::cout << "Compiling vertex shader from: vertex.hlsl" << std::endl;
    HRESULT hr = D3DCompileFromFile(
        L"vertex.hlsl",
        nullptr,
        nullptr,
        "main",
        "vs_5_1",
        0,
        0,
        &vertexShaderBlob,
        &errorBlob);
    
    if (FAILED(hr))
    {
        std::cerr << "Vertex shader compilation failed with HRESULT: 0x" << std::hex << hr << std::endl;
        if (errorBlob)
        {
            std::string errorMsg = (char*)errorBlob->GetBufferPointer();
            std::cerr << "Error: " << errorMsg << std::endl;
            errorBlob->Release();
        }
        else
        {
            std::cerr << "No error details available" << std::endl;
        }
        return false;
    }
    std::cout << "Vertex shader compiled successfully." << std::endl;
    
    m_vertexShaderBlob = vertexShaderBlob;
    
    // Compile pixel shader
    ID3DBlob* pixelShaderBlob = nullptr;
    errorBlob = nullptr;
    
    std::cout << "Compiling pixel shader from: pixel.hlsl" << std::endl;
    hr = D3DCompileFromFile(
        L"pixel.hlsl",
        nullptr,
        nullptr,
        "main",
        "ps_5_1",
        0,
        0,
        &pixelShaderBlob,
        &errorBlob);
    
    if (FAILED(hr))
    {
        std::cerr << "Pixel shader compilation failed with HRESULT: 0x" << std::hex << hr << std::endl;
        if (errorBlob)
        {
            std::string errorMsg = (char*)errorBlob->GetBufferPointer();
            std::cerr << "Error: " << errorMsg << std::endl;
            errorBlob->Release();
        }
        else
        {
            std::cerr << "No error details available" << std::endl;
        }
        return false;
    }
    std::cout << "Pixel shader compiled successfully." << std::endl;
    
    m_pixelShaderBlob = pixelShaderBlob;
    
    return true;
}

void GraphicsEngine::Cleanup()
{
    if (m_fenceEvent) CloseHandle(m_fenceEvent);
    
    if (m_fence) m_fence->Release();
    if (m_swapChain) m_swapChain->Release();
    if (m_commandList) m_commandList->Release();
    if (m_commandAllocator) m_commandAllocator->Release();
    if (m_commandQueue) m_commandQueue->Release();
    if (m_device) m_device->Release();
    if (m_adapter) m_adapter->Release();
    if (m_factory) m_factory->Release();
    
    // Cleanup graphics resources
    if (m_vertexBuffer) m_vertexBuffer->Release();
    if (m_pipelineState) m_pipelineState->Release();
    if (m_rootSignature) m_rootSignature->Release();
    if (m_vertexShaderBlob) m_vertexShaderBlob->Release();
    if (m_pixelShaderBlob) m_pixelShaderBlob->Release();
    
    for (int i = 0; i < 2; i++)
    {
        if (m_renderTargets[i]) m_renderTargets[i]->Release();
    }
}