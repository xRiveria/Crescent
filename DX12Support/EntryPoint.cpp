#include "Renderer/Renderer.h"
#include <memory>
#include "Window.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    std::unique_ptr<Crescent::Renderer> renderContext = std::make_unique<Crescent::Renderer>(1280, 720, L"Crescent (DX12)");
    
    return Crescent::Window::OnUpdate(renderContext.get(), hInstance, nCmdShow);
}