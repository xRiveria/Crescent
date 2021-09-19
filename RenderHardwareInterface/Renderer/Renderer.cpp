#include "RHI_PCH.h"
#include "Renderer.h"
#include "../RHI/RHI_Utilities.h"
#include <GLFW/glfw3.h>

namespace Aurora
{
    RenderAPI Renderer::s_CurrentRenderAPI = RenderAPI::Unknown;
    GLFWwindow* Renderer::s_RenderWindow;

    void Renderer::Initialize()
    {
        // Window Creation
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        s_RenderWindow = glfwCreateWindow(1280, 1080, ("Render Hardware Interface: " + RenderAPIToString(s_CurrentRenderAPI)).c_str(), nullptr, nullptr);

        // RHI
        m_RenderContext = RHI_Device::Create();
        m_RenderContext->Initialize();

        if (!m_RenderContext->IsInitialized())
        {
            std::cout << "Failed to initialize Renderer. Returning...";
            throw std::exception();
        }

        // Create Command Queue (DX12)


        // Create SwapChain
        m_SwapChain = RHI_SwapChain::Create(1280, 1080, s_RenderWindow, m_RenderContext.get(),
                                            m_SwapChainBufferCount, RHI_Present_Mode::RHI_Present_Immediate | RHI_Present_Mode::RHI_Swap_Flip_Discard,
                                            RHI_Format::RHI_Format_R8G8B8A8_Unorm, "Swapchain_Main");

        if (!m_SwapChain->IsInitialized())
        {
            std::cout << "Failed to create SwapChain.\n";
            return;
        }
            

        // Determine MSAA Support
    }

    void Renderer::SetRenderAPI(RenderAPI renderAPI)
    {
        s_CurrentRenderAPI = renderAPI;
        std::cout << "Render API Selected: " << RenderAPIToString(renderAPI) << "\n";
    }
}