#define NOMINMAX
#include <windows.h>
#include <stdio.h>
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
#include "../engine/rendering/camera/CameraSystem.h"
#include "../engine/rendering/camera/OrbitCameraController.h"
#include "../engine/rendering/camera/FirstPersonCameraController.h"
#include "../engine/rendering/camera/CinematicCameraController.h"
#include "../engine/rendering/culling/FrustumCuller.h"
#include "../engine/rendering/culling/SpatialPartition.h"
#include "../engine/rendering/lod/LODManager.h"
#include "../engine/rendering/projection/ProjectionMatrix.h"
#include "../engine/rendering/projection/ProjectionModes.h"

// Forward declarations for classes that are included above
// class GraphicsEngine;
// class Window;
// class GeneticsIntegration;
// class CameraSystem;

// Global camera system instance
std::unique_ptr<Engine::Rendering::CameraSystem> g_cameraSystem;

class Application
{
private:
    HINSTANCE m_hInstance;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<GraphicsEngine> m_graphicsEngine;
    std::unique_ptr<GeneticsIntegration> m_geneticsIntegration;
    std::unique_ptr<Engine::Rendering::CameraSystem> m_cameraSystem;
    
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

// Main entry point
int main()
{
    // No need for AllocConsole when using /SUBSYSTEM:CONSOLE
    
    try
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);
        Application app(hInstance);
        
        if (!app.Initialize())
        {
            std::cerr << "Failed to initialize application!" << std::endl;
            std::cerr.flush();
            return -1;
        }
        
        app.Run();
        app.Cleanup();
        
        std::cout << "Application exited successfully." << std::endl;
        std::cout.flush();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        std::cerr.flush();
        return -1;
    }
}

bool Application::Initialize()
{
    std::cout << "Initializing Application..." << std::endl;
    std::cout.flush();
    
    // Create window
    m_window = std::make_unique<Window>(m_hInstance);
    if (!m_window->Initialize(800, 600, L"3D Genetics Game"))
    {
        std::cerr << "Failed to initialize window!" << std::endl;
        std::cerr.flush();
        return false;
    }
    std::cout << "Window initialized successfully." << std::endl;
    std::cout.flush();
    
    // Give window time to fully initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Initialize graphics engine
    m_graphicsEngine = std::make_unique<GraphicsEngine>();
    if (!m_graphicsEngine->Initialize(m_window->GetHwnd()))
    {
        std::cerr << "Failed to initialize graphics engine!" << std::endl;
        std::cerr.flush();
        return false;
    }
    std::cout << "Graphics engine initialized successfully." << std::endl;
    std::cout.flush();
    
    // Initialize genetics integration
    m_geneticsIntegration = std::make_unique<GeneticsIntegration>();
    if (!m_geneticsIntegration->Initialize())
    {
        std::cerr << "Failed to initialize genetics integration!" << std::endl;
        std::cerr.flush();
        return false;
    }
    std::cout << "Genetics integration initialized successfully." << std::endl;
    std::cout.flush();
    
    // Initialize camera system
    std::cout << "Initializing camera system..." << std::endl;
    std::cout.flush();
    m_cameraSystem = std::make_unique<Engine::Rendering::CameraSystem>();
    if (m_cameraSystem)
    {
        // Create FPS camera with WASD + mouse controls
        auto fpsCamera = m_cameraSystem->CreateCamera<Engine::Rendering::FirstPersonCameraController>("fps");
        fpsCamera->SetPosition({0.0f, 2.0f, 10.0f});  // Start elevated and back
        fpsCamera->SetRotation({0.0f, 0.0f, 0.0f});   // Looking straight
        fpsCamera->SetSensitivity(0.002f);             // Mouse sensitivity
        m_cameraSystem->SetActiveCamera(fpsCamera);
        
        // Pass camera to window for input handling
        m_window->SetCamera(fpsCamera);
        
        std::cout << "Camera system initialized successfully." << std::endl;
        std::cout.flush();
    }
    else
    {
        std::cerr << "Failed to initialize camera system!" << std::endl;
        std::cerr.flush();
        return false;
    }
    
    // Initialize FPS counter
    m_lastFrameTime = std::chrono::steady_clock::now();
    
    std::cout << "Application initialization complete." << std::endl;
    std::cout.flush();
    return true;
}

void Application::Run()
{
    std::cout << "Starting message loop..." << std::endl;
    std::cout.flush();
    
    MSG msg = {};
    bool running = true;
    
    // Game loop using PeekMessage (non-blocking)
    while (running)
    {
        // Process all pending messages without blocking
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                std::cout << "Received WM_QUIT, exiting..." << std::endl;
                running = false;
                break;
            }
            
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (!running) break;
        
        float deltaTime = 0.0f;
        
        try
        {
            // Calculate delta time
            auto now = std::chrono::steady_clock::now();
            deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
            m_lastFrameTime = now;
            
            // Update systems
            m_graphicsEngine->Update();
            m_geneticsIntegration->Update(deltaTime);
            
            // Process keyboard input for camera movement
            if (m_window)
            {
                m_window->ProcessKeyboardInput(deltaTime);
            }
            
            // Update camera system
            if (m_cameraSystem)
            {
                m_cameraSystem->Update(deltaTime);
            }
            
            // Render everything in one pass
            m_graphicsEngine->Render(m_geneticsIntegration, m_cameraSystem ? m_cameraSystem->GetActiveCamera() : nullptr);
            
            // Update FPS counter
            UpdateFPS();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception during game loop: " << e.what() << std::endl;
            std::cerr.flush();
            break;
        }
        
        // REMOVED: Frame rate limiting to test responsiveness
        // if (deltaTime < 1.0f/60.0f)
        // {
        //     std::this_thread::sleep_for(std::chrono::microseconds(
        //         static_cast<long long>((1.0f/60.0f - deltaTime) * 1000000.0f))); 
        // }
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