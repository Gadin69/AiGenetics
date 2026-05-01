#pragma once

#include <windows.h>

class Window
{
private:
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    int m_width;
    int m_height;
    LPCSTR m_title;

public:
    Window(HINSTANCE hInstance) : m_hInstance(hInstance), m_hWnd(nullptr), m_width(0), m_height(0), m_title(nullptr) {}
    
    bool Initialize(int width, int height, LPCSTR title);
    HWND GetHwnd() const { return m_hWnd; }
    
private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};