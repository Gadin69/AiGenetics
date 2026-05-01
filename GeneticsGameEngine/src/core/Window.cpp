#include "Window.h"
#include <string>
#include <iostream>

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
        
        default:
            break;
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}