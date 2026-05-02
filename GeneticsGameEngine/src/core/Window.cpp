#include "Window.h"
#include "../engine/rendering/camera/CameraController.h"
#include "../engine/rendering/camera/OrbitCameraController.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <windowsx.h>

// Global window procedure pointer for static callback
LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Forward to window instance if available
    if (message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lpcs->lpCreateParams));
    }
    
    Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    
    if (window && message != WM_NCCREATE)
    {
        return window->HandleMessage(hWnd, message, wParam, lParam);
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool Window::Initialize(int width, int height, LPCWSTR title)
{
    std::cout << "Initializing window..." << std::endl;
    m_width = width;
    m_height = height;
    m_title = title;
    
    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW); // Set default cursor
    wc.lpszClassName = L"GeneticsGameWindowClass";
    
    if (!RegisterClassEx(&wc))
    {
        std::cerr << "Failed to register window class!" << std::endl;
        return false;
    }
    std::cout << "Window class registered successfully." << std::endl;
    
    // Create window
    m_hWnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        m_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        m_width, m_height,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );
    
    if (!m_hWnd)
    {
        std::cerr << "Failed to create window!" << std::endl;
        return false;
    }
    std::cout << "Window created successfully." << std::endl;
    
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
    
    std::cout << "Window shown and updated." << std::endl;
    return true;
}

LRESULT Window::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        
        case WM_SIZE:
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            break;
        
        case WM_LBUTTONDOWN:
            // Capture mouse when left button is clicked
            if (!m_mouseCaptured && m_camera)
            {
                SetCapture(hWnd);
                m_mouseCaptured = true;
                m_lastMouseX = GET_X_LPARAM(lParam);
                m_lastMouseY = GET_Y_LPARAM(lParam);
            }
            break;
        
        case WM_LBUTTONUP:
            // Release mouse capture
            if (m_mouseCaptured)
            {
                ReleaseCapture();
                m_mouseCaptured = false;
            }
            break;
        
        case WM_MOUSEMOVE:
            // Handle mouse movement for camera rotation
            if (m_mouseCaptured && m_camera)
            {
                int currentX = GET_X_LPARAM(lParam);
                int currentY = GET_Y_LPARAM(lParam);
                
                int deltaX = currentX - m_lastMouseX;
                int deltaY = currentY - m_lastMouseY;
                
                // Rotate camera based on mouse movement
                float sensitivity = 0.005f;
                m_camera->Rotate(deltaX * sensitivity, -deltaY * sensitivity);
                
                m_lastMouseX = currentX;
                m_lastMouseY = currentY;
            }
            break;
        
        case WM_MOUSEWHEEL:
            // Handle mouse wheel for zoom
            if (m_camera)
            {
                int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                float zoomSensitivity = 0.5f;
                
                // Try to cast to OrbitCameraController for zoom
                auto orbitCamera = dynamic_cast<Engine::Rendering::OrbitCameraController*>(m_camera);
                if (orbitCamera)
                {
                    float currentDistance = orbitCamera->GetDistance();
                    float newDistance = currentDistance - (wheelDelta > 0 ? zoomSensitivity : -zoomSensitivity);
                    if (newDistance < 1.0f) newDistance = 1.0f;
                    orbitCamera->SetDistance(newDistance);
                }
            }
            break;
        
        case WM_KEYDOWN:
            // Handle keyboard input for camera movement
            if (m_camera)
            {
                float moveSpeed = 0.5f;
                
                switch (wParam)
                {
                    case 'W':
                        m_camera->MoveForward(moveSpeed);
                        break;
                    case 'S':
                        m_camera->MoveForward(-moveSpeed);
                        break;
                    case 'A':
                        m_camera->MoveRight(-moveSpeed);
                        break;
                    case 'D':
                        m_camera->MoveRight(moveSpeed);
                        break;
                    case 'Q':
                        m_camera->MoveUp(-moveSpeed);
                        break;
                    case 'E':
                        m_camera->MoveUp(moveSpeed);
                        break;
                }
            }
            break;
        
        default:
            break;
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}