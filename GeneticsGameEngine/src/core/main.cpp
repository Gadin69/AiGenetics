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

// Global crash handler to catch access violations
LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
    std::cerr << "\n=== CRASH DETECTED ==="  << std::endl;
    std::cerr << "Exception code: 0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionCode << std::dec << std::endl;
    std::cerr << "Exception address: 0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionAddress << std::dec << std::endl;
    std::cerr.flush();
    
    return EXCEPTION_EXECUTE_HANDLER;
}

// Console control handler to catch termination signals
BOOL WINAPI ConsoleCtrlHandler(DWORD fdwCtrlType)
{
    std::cout << "[MAIN] Console control signal received: " << fdwCtrlType << std::endl;
    std::cout.flush();
    return FALSE; // Let system handle it
}

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
    std::chrono::steady_clock::time_point m_startTime;
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
    // Register crash handler FIRST
    SetUnhandledExceptionFilter(CrashHandler);
    
    // Register console control handler
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    
    // No need for AllocConsole when using /SUBSYSTEM:CONSOLE
    
    try
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);
        Application app(hInstance);
        
        if (!app.Initialize())
        {
            std::cerr << "Failed to initialize application!" << std::endl;
            std::cerr.flush();
            std::cout << "\nPress any key to exit..." << std::endl;
            std::cin.get();
            return -1;
        }
        
        app.Run();
        app.Cleanup();
        
        std::cout << "\n=== Application Closed ==="  << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get();
        
        std::cout << "Application exited successfully." << std::endl;
        std::cout.flush();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        std::cerr.flush();
        std::cout << "\nPress any key to exit..." << std::endl;
        std::cin.get();
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
    
    // Set graphics engine pointer in window for input handling
    m_window->SetGraphicsEngine(m_graphicsEngine.get());
    
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
    m_startTime = m_lastFrameTime;
    
    // Phase 3: Generate creature meshes from genetics
    std::cout << "\n=== Phase 3: Procedural Mesh Generation ===" << std::endl;
    try {
        // Phase 5: Initialize PBR material system
        std::cout << "\n=== Phase 5: PBR Material System ===" << std::endl;
        m_geneticsIntegration->InitializeMaterialSystem(m_graphicsEngine->GetDevice());
        
        m_geneticsIntegration->GenerateCreatureMeshes(
            m_graphicsEngine->GetDevice(),
            m_graphicsEngine->GetCommandList()
        );
        std::cout << "Application initialization complete." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception during mesh generation: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception during mesh generation!" << std::endl;
        return false;
    }
    std::cout.flush();
    return true;
}

void Application::Run()
{
    std::cout << "[MAIN] Starting message loop..." << std::endl;
    std::cout.flush();
    
    MSG msg = {};
    bool running = true;
    int frameCount = 0;
    
    // Game loop using PeekMessage (non-blocking)
    while (running)
    {
        // Process all pending messages without blocking
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                std::cout << "[MAIN] Received WM_QUIT, exiting..." << std::endl;
                running = false;
                break;
            }
            
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (!running) break;
        
        frameCount++;
        if (frameCount % 100 == 0) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();
            std::cout << "[MAIN] Frame count: " << frameCount << " | Elapsed: " << elapsed << "s" << std::endl;
            std::cout.flush();
        }
        
        float deltaTime = 0.0f;
        
        try
        {
            // Calculate delta time
            auto now = std::chrono::steady_clock::now();
            deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
            m_lastFrameTime = now;
            
            // Update systems with null checks
            if (m_graphicsEngine)
            {
                m_graphicsEngine->Update();
            }
            else
            {
                std::cerr << "[ERROR] m_graphicsEngine is null!" << std::endl;
            }
            
            if (m_geneticsIntegration)
            {
                m_geneticsIntegration->Update(deltaTime);
            }
            else
            {
                std::cerr << "[ERROR] m_geneticsIntegration is null!" << std::endl;
            }
            
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
            std::cerr << "[EXCEPTION] Standard exception during game loop: " << e.what() << std::endl;
            std::cerr.flush();
            break;
        }
        catch (...)
        {
            std::cerr << "[CRITICAL] Unknown exception during game loop - application will terminate" << std::endl;
            std::cerr.flush();
            break;
        }
        
        // Enable frame rate limiting to prevent GPU overload
        if (deltaTime < 1.0f/60.0f)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(
                static_cast<long long>((1.0f/60.0f - deltaTime) * 1000000.0f))); 
        }
    }
    
    std::cout << "[MAIN] Message loop ended, total frames: " << frameCount << std::endl;
    std::cout.flush();
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
    std::cout << "[MAIN] Cleaning up application..." << std::endl;
    std::cout.flush();
    
    m_geneticsIntegration.reset();
    m_graphicsEngine.reset();
    m_window.reset();
    
    std::cout << "[MAIN] Cleanup complete" << std::endl;
    std::cout.flush();
}


