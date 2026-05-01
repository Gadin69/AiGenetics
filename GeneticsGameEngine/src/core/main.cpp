#include <windows.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include "dx12_test.h"

// Include actual header files (not just forward declarations)
#include "../graphics/GraphicsEngine.h"
#include "../core/Window.h"
#include "../genetics/GeneticsIntegration.h"

// Forward declarations for classes that are included above
// class GraphicsEngine;
// class Window;
// class GeneticsIntegration;

class Application
{
private:
    HINSTANCE m_hInstance;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<GraphicsEngine> m_graphicsEngine;
    std::unique_ptr<GeneticsIntegration> m_geneticsIntegration;
    
    // FPS counter
    std::chrono::steady_clock::time_point m_lastFrameTime;
    int m_frameCount;
    float m_fps;

public:
    Application(HINSTANCE hInstance) : m_hInstance(hInstance), m_frameCount(0), m_fps(0.0f) {}
    
    bool Initialize();
    void Run();
    void Cleanup();
    
private:
    void UpdateFPS();
};

// WinMain entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        Application app(hInstance);
        
        if (!app.Initialize())
        {
            std::cerr << "Failed to initialize application!" << std::endl;
            return -1;
        }
        
        app.Run();
        app.Cleanup();
        
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
}

bool Application::Initialize()
{
    // Create window
    m_window = std::make_unique<Window>(m_hInstance);
    if (!m_window->Initialize(1920, 1080, L"3D Genetics Game"))
    {
        return false;
    }
    
    // Initialize graphics engine
    m_graphicsEngine = std::make_unique<GraphicsEngine>();
    if (!m_graphicsEngine->Initialize(m_window->GetHwnd()))
    {
        return false;
    }
    
    // Initialize genetics integration
    m_geneticsIntegration = std::make_unique<GeneticsIntegration>();
    if (!m_geneticsIntegration->Initialize())
    {
        return false;
    }
    
    // Add a test creature
    m_geneticsIntegration->AddCreature("Chordata", "test_creature_001");
    
    // Initialize FPS counter
    m_lastFrameTime = std::chrono::steady_clock::now();
    
    return true;
}

void Application::Run()
{
    MSG msg = {};
    
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Calculate delta time
            auto now = std::chrono::steady_clock::now();
            auto deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
            m_lastFrameTime = now;
            
            // Update systems
            m_graphicsEngine->Update();
            m_geneticsIntegration->Update(deltaTime);
            
            // Render
            m_graphicsEngine->Render();
            m_geneticsIntegration->Render(m_graphicsEngine.get());
            
            // Update FPS counter
            UpdateFPS();
            
            // Cap frame rate at 60 FPS
            if (deltaTime < 1.0f/60.0f)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(
                    static_cast<long long>((1.0f/60.0f - deltaTime) * 1000000.0f)));
            }
        }
    }
}

void Application::UpdateFPS()
{
    m_frameCount++;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<float>(now - m_lastFrameTime).count();
    
    if (elapsed >= 1.0f)
    {
        m_fps = m_frameCount / elapsed;
        m_frameCount = 0;
        m_lastFrameTime = now;
        
        // Print FPS to console
        std::cout << "FPS: " << static_cast<int>(m_fps) << std::endl;
    }
}

void Application::Cleanup()
{
    m_geneticsIntegration.reset();
    m_graphicsEngine.reset();
    m_window.reset();
}