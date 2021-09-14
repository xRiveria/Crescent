#pragma once
#include <Windows.h>

namespace Crescent
{
    class Renderer;

    class Window
    {
    public:
        static int OnUpdate(Renderer* renderer, HINSTANCE hInstance, int nCmdShow); 
        static HWND GetHWND() { return m_HWND; }

    protected:
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    private:
        static HWND m_HWND;
    };
}