#pragma once

#include <windows.h>

// Forward declarations for engine classes
namespace Engine {
    namespace Rendering {
        class BaseCameraController;
    }
}

class Window
{
private:
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    int m_width;
    int m_height;
    LPCWSTR m_title;

public:
    Window(HINSTANCE hInstance) : m_hInstance(hInstance), m_hWnd(nullptr), m_width(0), m_height(0), m_title(nullptr) {}
    
    bool Initialize(int width, int height, LPCWSTR title);
    HWND GetHwnd() const { return m_hWnd; }
    
private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};