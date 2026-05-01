#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <windows.h>
#include <vector>
#include <memory>

// Forward declarations
class GeneticsIntegration;

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
    IDXGISwapChain1* m_swapChain;
    IDXGISwapChain3* m_swapChain3;
    ID3D12Resource* m_renderTargets[2];
    UINT m_frameIndex;
    
    // Synchronization
    HANDLE m_fenceEvent;
    ID3D12Fence* m_fence;
    UINT64 m_fenceValue;
    
public:
    GraphicsEngine() : m_hWnd(nullptr), m_device(nullptr), m_adapter(nullptr), 
        m_factory(nullptr), m_commandQueue(nullptr), m_commandAllocator(nullptr), 
        m_commandList(nullptr), m_swapChain(nullptr), m_swapChain3(nullptr), m_frameIndex(0), 
        m_fenceEvent(nullptr), m_fence(nullptr), m_fenceValue(0) {}
    
    bool Initialize(HWND hWnd);
    void Cleanup();
    
    void Update();
    void Render();
    
private:
    bool InitializeDX12();
    bool CreateSwapChain();
    bool CreateCommandObjects();
    bool CreateSyncObjects();
    
    void WaitForPreviousFrame();
    void MoveToNextFrame();
};