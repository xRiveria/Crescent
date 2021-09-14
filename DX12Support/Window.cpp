#include "Window.h"
#include "Renderer/Renderer.h"

namespace Crescent
{
    HWND Window::m_HWND = nullptr;

    int Window::OnUpdate(Renderer* renderer, HINSTANCE hInstance, int nCmdShow)
    {
        // Initialize window class.
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = L"CrescentClass";
        RegisterClassEx(&windowClass);

        RECT windowRect = { 0, 0, static_cast<LONG>(renderer->GetWindowWidth()), static_cast<LONG>(renderer->GetWindowHeight()) };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        // Create the window and store a handle to it.
        m_HWND = CreateWindow(
            windowClass.lpszClassName,
            renderer->GetWindowName().c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr, // We have no parent window.
            nullptr, // We aren't using menus.
            hInstance,
            renderer);

        // Initialize our renderer.
        renderer->OnInitialize();

        ShowWindow(m_HWND, nCmdShow);

        // Main sample loop.
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            // Process any messages in the queue.
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        renderer->OnDestroy();

        // Return this part of the WM_QUIT message to Windows.
        return static_cast<char>(msg.wParam);
    }

    LRESULT Window::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        Renderer* renderer = reinterpret_cast<Renderer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (message)
        {
            case WM_CREATE:
            {
                // Save the Renderer* passed in to CreateWindow.
                LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            }
            return 0;

            case WM_KEYDOWN:
            {

            }
            return 0;

            case WM_KEYUP:
            {

            }
            return 0;

            case WM_PAINT:
            {
                if (renderer)
                {
                    renderer->OnUpdate();
                    renderer->OnRender();
                }
            }
            return 0;

            case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            }
        }

        // Handle any messages the switch statement didn't.
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}