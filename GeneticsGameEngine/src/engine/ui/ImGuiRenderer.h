#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <windows.h>
#include <wrl/client.h>
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

class ImGuiRenderer
{
public:
    ImGuiRenderer();
    ~ImGuiRenderer();

    // Initialize ImGui with DX12 device and swap chain
    bool Initialize(ID3D12Device* device,
                    ID3D12CommandQueue* commandQueue,
                    ID3D12DescriptorHeap* rtvHeap,
                    ID3D12DescriptorHeap* dsvHeap,
                    IDXGISwapChain3* swapChain,
                    UINT numBackBuffers,
                    UINT width,
                    UINT height);

    // Shutdown ImGui
    void Shutdown();

    // Start new ImGui frame (call at beginning of frame)
    void NewFrame();

    // Render ImGui draw data (call at end of frame, after main scene)
    void RenderDrawData(ID3D12GraphicsCommandList* commandList);

    // Get ImGui's descriptor heap (must be set before rendering ImGui)
    ID3D12DescriptorHeap* GetDescriptorHeap() const { return m_srvUavHeap.Get(); }

    // Handle Windows messages for ImGui input
    static LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvUavHeap; // SRV/UAV heap for ImGui textures
};
