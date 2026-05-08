#include "ImGuiRenderer.h"
#include <iostream>

// Forward declare ImGui Win32 message handler (not exposed in header by default)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// External window handle (set by main.cpp)
extern HWND g_hWnd;

ImGuiRenderer::ImGuiRenderer()
{
}

ImGuiRenderer::~ImGuiRenderer()
{
    Shutdown();
}

bool ImGuiRenderer::Initialize(ID3D12Device* device,
                                ID3D12CommandQueue* commandQueue,
                                ID3D12DescriptorHeap* rtvHeap,
                                ID3D12DescriptorHeap* dsvHeap,
                                IDXGISwapChain3* swapChain,
                                UINT numBackBuffers,
                                UINT width,
                                UINT height)
{
    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Create SRV/UAV descriptor heap for ImGui
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1024; // Allow many textures
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvUavHeap))))
    {
        std::cerr << "Failed to create ImGui SRV/UAV heap" << std::endl;
        return false;
    }

    // Initialize ImGui backends - use struct-based API
    ImGui_ImplWin32_Init(g_hWnd);
    
    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device = device;
    initInfo.CommandQueue = commandQueue;
    initInfo.NumFramesInFlight = numBackBuffers;
    initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    initInfo.SrvDescriptorHeap = m_srvUavHeap.Get();
    initInfo.LegacySingleSrvCpuDescriptor = m_srvUavHeap.Get()->GetCPUDescriptorHandleForHeapStart();
    initInfo.LegacySingleSrvGpuDescriptor = m_srvUavHeap.Get()->GetGPUDescriptorHandleForHeapStart();
    
    if (!ImGui_ImplDX12_Init(&initInfo))
    {
        std::cerr << "Failed to initialize ImGui DX12 backend" << std::endl;
        return false;
    }

    std::cout << "ImGui initialized successfully" << std::endl;
    return true;
}

void ImGuiRenderer::Shutdown()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiRenderer::NewFrame()
{
    // Start new frame - correct order per ImGui documentation
    ImGui_ImplWin32_NewFrame(); // 1. Process Win32 input
    ImGui_ImplDX12_NewFrame();  // 2. Reset DX12 state (builds font texture on first call)
    ImGui::NewFrame();          // 3. Start ImGui frame
}

void ImGuiRenderer::RenderDrawData(ID3D12GraphicsCommandList* commandList)
{
    // Render ImGui
    ImGui::Render();
    
    // Set descriptor heap for ImGui
    ID3D12DescriptorHeap* heaps[] = { m_srvUavHeap.Get() };
    commandList->SetDescriptorHeaps(1, heaps);
    
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

LRESULT ImGuiRenderer::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}
