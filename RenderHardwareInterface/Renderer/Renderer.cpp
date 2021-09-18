#include "Renderer.h"
#include "../RHI/RHI_Utilities.h"
#include <iostream>

namespace Aurora
{
    RenderAPI Renderer::s_CurrentRenderAPI = RenderAPI::Unknown;

    void Renderer::Initialize()
    {
        m_RenderContext = RHI_Device::Create();
        m_RenderContext->Initialize();

        if (!m_RenderContext->IsInitialized())
        {
            std::cout << "Failed to initialize Renderer. Returning...";
            throw std::exception();
        }
    }

    void Renderer::SetRenderAPI(RenderAPI renderAPI)
    {
        s_CurrentRenderAPI = renderAPI;
        std::cout << "Render API Selected: " << RenderAPIToString(renderAPI) << "\n";
    }
}