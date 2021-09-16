#pragma once
#include "../RHI/RHI_Device.h"

namespace Aurora
{
    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer() {}

        void Initialize();
        void SetupStates();
        void CreateDefaultResources();

        // API
        static RenderAPI GetCurrentRenderAPI() { return s_CurrentRenderAPI; }
        static void SetRenderAPI(RenderAPI renderAPI);

    private:
        std::shared_ptr<RHI_Device> m_RenderContext;

        static RenderAPI s_CurrentRenderAPI; // Initialized to Unknown.
    };
}