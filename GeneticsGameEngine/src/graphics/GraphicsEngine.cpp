#include "GraphicsEngine.h"
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <windows.h>
#include <iostream>
#include <d3dx12.h>

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
    if (!InitializeDX12())
    {
        return false;
    }
    
    // Create swap chain
    if (!CreateSwapChain())
    {
        return false;
    }
    
    // Create command objects
    if (!CreateCommandObjects())
    {
        return false;
    }
    
    // Create synchronization objects
    if (!CreateSyncObjects())
    {
        return false;
    }
    
    return true;
}

bool GraphicsEngine::InitializeDX12()
{
    // Create DXGI factory
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)));
    
    // Create adapter (GPU)
    ThrowIfFailed(m_factory->EnumAdapters1(0, &m_adapter));
    
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
    // Get window size
    RECT clientRect;
    GetClientRect(m_hWnd, &clientRect);
    UINT width = clientRect.right - clientRect.left;
    UINT height = clientRect.bottom - clientRect.top;
    
    // Create swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Stereo = FALSE;
    
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc = {};
    fsDesc.RefreshRate.Numerator = 60;
    fsDesc.RefreshRate.Denominator = 1;
    fsDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    fsDesc.Rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
    fsDesc.Windowed = TRUE;
    
    ThrowIfFailed(m_factory->CreateSwapChainForHwnd(
        m_commandQueue,
        m_hWnd,
        &swapChainDesc,
        &fsDesc,
        nullptr,
        &m_swapChain
    ));
    
    // Create render target views
    for (int i = 0; i < 2; i++)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
        
        // Create RTV descriptor heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 2;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        
        ID3D12DescriptorHeap* rtvHeap;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));
        
        // Create RTV
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);
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
    
    return true;
}

bool GraphicsEngine::CreateSyncObjects()
{
    // Create fence
    ThrowIfFailed(m_device->CreateFence(
        0,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&m_fence)
    ));
    
    // Create fence event
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        return false;
    }
    
    return true;
}

void GraphicsEngine::Update()
{
    // Update logic here
}

void GraphicsEngine::Render()
{
    // Wait for previous frame
    WaitForPreviousFrame();
    
    // Reset command allocator
    m_commandAllocator->Reset();
    
    // Reset command list
    m_commandList->Reset(m_commandAllocator, nullptr);
    
    // Set render target
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[m_frameIndex],
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    m_commandList->ResourceBarrier(1, &barrier);
    
    // Clear render target
    float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    
    // Transition back to present state
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[m_frameIndex],
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
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
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
    
    m_fenceValue++;
}

void GraphicsEngine::MoveToNextFrame()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
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
    
    for (int i = 0; i < 2; i++)
    {
        if (m_renderTargets[i]) m_renderTargets[i]->Release();
    }
}