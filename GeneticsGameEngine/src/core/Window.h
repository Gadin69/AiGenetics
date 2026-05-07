#pragma once

#include <windows.h>
#include <memory>

// Forward declarations for engine classes
namespace Engine {
    namespace Rendering {
        class BaseCameraController;
    }
}

// Forward declaration
class GraphicsEngine;

class Window
{
private:
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    int m_width;
    int m_height;
    LPCWSTR m_title;
    
    // Camera pointer for input handling
    Engine::Rendering::BaseCameraController* m_camera;
    
    // Graphics engine pointer for wireframe toggle
    GraphicsEngine* m_graphicsEngine;
    
    // Mouse state
    bool m_mouseCaptured;
    int m_lastMouseX;
    int m_lastMouseY;
    
    // Keyboard state for WASD
    bool m_keyW;
    bool m_keyA;
    bool m_keyS;
    bool m_keyD;

public:
    Window(HINSTANCE hInstance) : m_hInstance(hInstance), m_hWnd(nullptr), m_width(0), m_height(0), m_title(nullptr), 
                                   m_camera(nullptr), m_graphicsEngine(nullptr), m_mouseCaptured(false), m_lastMouseX(0), m_lastMouseY(0),
                                   m_keyW(false), m_keyA(false), m_keyS(false), m_keyD(false) {}
    
    bool Initialize(int width, int height, LPCWSTR title);
    HWND GetHwnd() const { return m_hWnd; }
    void SetCamera(Engine::Rendering::BaseCameraController* camera) { m_camera = camera; }
    void SetGraphicsEngine(GraphicsEngine* engine) { m_graphicsEngine = engine; }
    void ProcessKeyboardInput(float deltaTime);
    
private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};