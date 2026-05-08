#include "Window.h"
#include "../engine/rendering/camera/CameraController.h"
#include "../engine/rendering/camera/OrbitCameraController.h"
#include "../engine/ui/ImGuiRenderer.h"
#include "../graphics/GraphicsEngine.h"
#include "../../third_party/imgui/imgui.h"
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

void Window::ProcessKeyboardInput(float deltaTime)
{
    if (!m_camera)
        return;
    
    float moveSpeed = 10.0f * deltaTime;  // Units per second
    
    if (m_keyW) {
        // std::cout << "[INPUT] W pressed - calling MoveForward(" << moveSpeed << ")" << std::endl;
        m_camera->MoveForward(moveSpeed);
    }
    if (m_keyS) {
        // std::cout << "[INPUT] S pressed - moving backward " << moveSpeed << " units" << std::endl;
        m_camera->MoveForward(-moveSpeed);
    }
    if (m_keyA) {
        // std::cout << "[INPUT] A pressed - moving left " << moveSpeed << " units" << std::endl;
        m_camera->MoveRight(-moveSpeed);
    }
    if (m_keyD) {
        // std::cout << "[INPUT] D pressed - moving right " << moveSpeed << " units" << std::endl;
        m_camera->MoveRight(moveSpeed);
    }
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
    // Forward to ImGui first
    if (ImGuiRenderer::MsgProc(hWnd, message, wParam, lParam))
    {
        return true;
    }
    
    switch (message)
    {
        case WM_DESTROY:
            std::cout << "[Window] WM_DESTROY received - posting quit message" << std::endl;
            PostQuitMessage(0);
            return 0;
        
        case WM_SIZE:
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            break;
        
        case WM_LBUTTONDOWN:
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                
                // Check if ImGui wants to capture mouse input
                if (ImGui::GetIO().WantCaptureMouse)
                {
                    // Don't capture mouse for camera - let ImGui handle it
                    break;
                }
                
                // Forward to graphics engine for UI button click
                if (m_graphicsEngine)
                {
                    m_graphicsEngine->OnMouseClick(x, y);
                    
                    // Don't capture mouse if clicking on UI button
                    if (m_graphicsEngine->IsPointInButton(x, y))
                    {
                        break; // Skip camera rotation
                    }
                }
                
                // Start mouse capture for camera rotation
                SetCapture(hWnd);
                m_mouseCaptured = true;
                m_lastMouseX = x;
                m_lastMouseY = y;
                
                // Hide cursor for FPS camera control
                ShowCursor(FALSE);
            }
            break;
        
        case WM_MOUSELEAVE:
            if (m_graphicsEngine)
            {
                m_graphicsEngine->OnMouseLeave();
            }
            break;
        
        case WM_LBUTTONUP:
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                
                // Forward to graphics engine for UI button release
                if (m_graphicsEngine)
                {
                    // Check if releasing on button
                }
                
                // Release mouse capture
                if (m_mouseCaptured)
                {
                    ReleaseCapture();
                    m_mouseCaptured = false;
                    ShowCursor(TRUE);
                }
            }
            break;
        
        case WM_MOUSEMOVE:
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                
                // Check if ImGui wants to capture mouse input
                if (ImGui::GetIO().WantCaptureMouse)
                {
                    // Don't rotate camera - let ImGui handle mouse
                    m_lastMouseX = x;
                    m_lastMouseY = y;
                    break;
                }
                
                // Forward to graphics engine for UI button hover detection
                if (m_graphicsEngine)
                {
                    m_graphicsEngine->OnMouseMove(x, y);
                }
                
                // Handle mouse movement for camera rotation
                if (m_mouseCaptured && m_camera)
                {
                    int deltaX = x - m_lastMouseX;
                    int deltaY = y - m_lastMouseY;
                    
                    // CAMERA MOVEMENT - DO NOT MODIFY unless explicitly requested
                    // Mouse rotation with both axes negated for correct direction:
                    // -deltaX: Moving mouse right (positive delta) rotates camera right (positive yaw)
                    // -deltaY: Moving mouse up (negative delta) rotates camera up (positive pitch)
                    float sensitivity = 0.005f;
                    m_camera->Rotate(-deltaX * sensitivity, -deltaY * sensitivity);
                    
                    m_lastMouseX = x;
                    m_lastMouseY = y;
                }
                else
                {
                    m_lastMouseX = x;
                    m_lastMouseY = y;
                }
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
            // Track key states for continuous movement
            switch (wParam)
            {
                case 'W': m_keyW = true; break;
                case 'S': m_keyS = true; break;
                case 'A': m_keyA = true; break;
                case 'D': m_keyD = true; break;
                case '1': // Toggle wireframe mode
                    std::cout << "[DEBUG] '1' key pressed!" << std::endl;
                    if (m_graphicsEngine)
                    {
                        m_graphicsEngine->ToggleWireframe();
                        std::cout << "[DEBUG] Wireframe toggled to: " 
                                  << (m_graphicsEngine->IsWireframeMode() ? "ON" : "OFF") << std::endl;
                    }
                    else
                    {
                        std::cout << "[ERROR] GraphicsEngine pointer is null!" << std::endl;
                    }
                    break;
            }
            break;
        
        case WM_KEYUP:
            // Reset key states
            switch (wParam)
            {
                case 'W': m_keyW = false; break;
                case 'S': m_keyS = false; break;
                case 'A': m_keyA = false; break;
                case 'D': m_keyD = false; break;
            }
            break;
        
        default:
            break;
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}