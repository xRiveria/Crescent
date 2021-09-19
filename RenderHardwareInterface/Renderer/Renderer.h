#pragma once
#include "../RHI/RHI_Device.h"
#include "../RHI/RHI_SwapChain.h"

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

        // API
        static RenderAPI GetCurrentRenderAPI() { return s_CurrentRenderAPI; }
        static void SetRenderAPI(RenderAPI renderAPI);

    public:
        static GLFWwindow* s_RenderWindow;

    private:
        std::shared_ptr<RHI_Device> m_RenderContext;
        // SwapChain
        static const uint8_t m_SwapChainBufferCount = 3;
        std::shared_ptr<RHI_SwapChain> m_SwapChain;

        static RenderAPI s_CurrentRenderAPI; // Initialized to Unknown.
    };
}