#pragma once
#include "../RHI/RHI_Device.h"
#include "../RHI/RHI_SwapChain.h"
#include "../RHI/RHI_Texture.h"
#include "../RHI/RHI_States.h"
#include "../RHI/RHI_CommandList.h"
#include "../Math/Vector2.h"
#include <array>

struct GLFWwindow;

namespace Aurora
{
    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer() {}

        void Initialize();
        // void SetupStates();
        // void CreateDefaultResources();

        // Resolution
        void SetResolutionRender(uint32_t newWidth, uint32_t newHeight, bool recreateResources = true);

        // Viewport
        void SetViewport(float newWidth, float newHeight);

        // API
        static RenderAPI GetCurrentRenderAPI() { return s_CurrentRenderAPI; }
        static void SetRenderAPI(RenderAPI renderAPI);

    public:
        static GLFWwindow* s_RenderWindow;

    private:
        // Resource Creation
        void CreateFramebuffers(const bool createRenderBuffers = true);
        void CreateAssets();

    private:
        std::shared_ptr<RHI_Device> m_RenderContext;

        // Render Textures
        enum Renderer_RenderTarget : uint32_t
        {
            ColorBuffer,
            DepthBuffer,
        };
        std::array<std::shared_ptr<RHI_Texture>, 2> m_RenderTargets;
        #define RenderTarget(enumIndex) m_RenderTargets[static_cast<uint8_t>(enumIndex)]

        // SwapChain
        Vector2 m_ResolutionRender;
        static const uint8_t m_SwapChainBufferCount = 3;
        std::shared_ptr<RHI_SwapChain> m_SwapChain;
        std::shared_ptr<RHI_CommandList> m_CommandList;

        // Rasterizer States
        std::shared_ptr<RHI_RasterizerState> m_RasterizerState_CullBackSolid;

        // Blend States
        std::shared_ptr<RHI_BlendState> m_BlendState_Disabled;

        // Depth Stencil States
        std::shared_ptr<RHI_DepthStencilState> m_DepthStencilState_ReadWrite_Off;

        static RenderAPI s_CurrentRenderAPI; // Initialized to Unknown.
    };
}