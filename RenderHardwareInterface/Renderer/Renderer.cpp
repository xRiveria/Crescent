#include "RHI_PCH.h"
#include "../RHI/API_Utilities/RHI_Display.h"
#include "Renderer.h"
#include "../RHI/RHI_Utilities.h"
#include "../RHI/RHI_Texture2D.h"
#include <GLFW/glfw3.h>

namespace Aurora
{
    RenderAPI Renderer::s_CurrentRenderAPI = RenderAPI::Unknown;
    GLFWwindow* Renderer::s_RenderWindow;

    void Renderer::Initialize()
    {
        // Window Creation
        const int windowWidth = 1280;
        const int windowHeight = 720;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        s_RenderWindow = glfwCreateWindow(windowWidth, windowHeight, ("Render Hardware Interface: " + RenderAPIToString(s_CurrentRenderAPI)).c_str(), nullptr, nullptr);

        // RHI
        m_RenderContext = RHI_Device::Create();
        m_RenderContext->Initialize();

        if (!m_RenderContext->IsInitialized())
        {
            std::cout << "Failed to initialize Renderer. Returning...";
            throw std::exception();
        }

        /// Create Command Queue (DX12)


        // Create SwapChain
        m_SwapChain = RHI_SwapChain::Create(windowWidth, windowHeight, s_RenderWindow, m_RenderContext.get(),
                                            m_SwapChainBufferCount, RHI_Present_Mode::RHI_Present_Immediate | RHI_Present_Mode::RHI_Swap_Flip_Discard,
                                            RHI_Format::RHI_Format_R8G8B8A8_Unorm, "Swapchain_Main");

        if (!m_SwapChain->IsInitialized())
        {
            std::cout << "Failed to create SwapChain.\n";
            return;
        }

        // Determine MSAA Support

        // Set render resolutions/size to whatever the window is (initially).
        SetResolutionRender(windowWidth, windowHeight, false);
        // SetViewport(static_cast<float>(windowWidth), static_cast<float>(windowHeight));

        // Create Resources
        CreateFramebuffers(true);
    }

    void Renderer::SetResolutionRender(uint32_t width, uint32_t height, bool recreateResources /*= true*/)
    {
        // Check if the requested resolution is valid.
        if (!RHI_Device::IsValidResolution(width, height))
        {
            std::cout << "Requested resolution change to " << width << ", " << height << " rejected.\n";
            return;
        }

        // Ensure that we are pixel perfect.
        width  -= (width  % 2 != 0) ? 1 : 0;
        height -= (height % 2 != 0) ? 1 : 0;
        
        // Return if the resolution has already been set.
        if (m_ResolutionRender.x == width && m_ResolutionRender.y == height)
        {
            return;
        }

        // Set Resolution
        m_ResolutionRender.x = static_cast<float>(width);
        m_ResolutionRender.y = static_cast<float>(height);

        // Set as the currently active display mode.
        RHI_DisplayMode displayMode = RHI_Display::GetActiveDisplayMode();
        displayMode.SetWidth(width);
        displayMode.SetHeight(height);
        RHI_Display::SetActiveDisplayMode(displayMode);

        // Register display mode (in case it doesn't exist), but maintain the FPS limit.
        bool updateFPSLimitToHighestHertz = false;
        RHI_Display::RegisterDisplayMode(RHI_DisplayMode(displayMode.m_Width, displayMode.m_Height, displayMode.m_Numerator, displayMode.m_Denominator), updateFPSLimitToHighestHertz);
        
        if (recreateResources)
        {
            /// Recreate Textures & Samplers
        }

        // Log
        std::cout << "Render Resolution has been set to " << width << ", " << height << "\n";
    }

    void Renderer::SetViewport(float newWidth, float newHeight)
    {

    }

    void Renderer::SetRenderAPI(RenderAPI renderAPI)
    {
        s_CurrentRenderAPI = renderAPI;
        std::cout << "Render API Selected: " << RenderAPIToString(renderAPI) << "\n";
    }

    void Renderer::CreateFramebuffers(const bool createRenderBuffers)
    {
        // Get current render resolution.
        uint32_t renderWidth = static_cast<uint32_t>(m_ResolutionRender.x);
        uint32_t renderHeight = static_cast<uint32_t>(m_ResolutionRender.y);

        /// We ought to flush the GPU before recreating our render textures.

        // Mip Count
        uint32_t mipCount = 1;

        if (createRenderBuffers)
        {
            RenderTarget(Renderer_RenderTarget::ColorBuffer) = RHI_Texture2D::Create(renderWidth, renderHeight, 1, RHI_Format::RHI_Format_R11G11B10_Float,
                                                                                     RHI_Texture_Flag::RHI_Texture_RT_Color | RHI_Texture_Flag::RHI_Texture_SRV,
                                                                                     "RT_ColorBuffer");

            RenderTarget(Renderer_RenderTarget::DepthBuffer) = RHI_Texture2D::Create(renderWidth, renderHeight, 1, RHI_Format::RHI_Format_D32_Float,
                                                                                     RHI_Texture_Flag::RHI_Texture_RT_DepthStencil | RHI_Texture_Flag::RHI_Texture_SRV, 
                                                                                     "RT_DepthBuffer");
        }
    }
}