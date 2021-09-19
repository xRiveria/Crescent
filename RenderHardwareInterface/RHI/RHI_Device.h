#pragma once
#include <memory>
#include <vector>
#include "API_Utilities/RHI_GPU.h"

namespace Aurora
{
    enum class RenderAPI
    {
        Unknown,
        DirectX11,
        DirectX12,
        Vulkan,
        OpenGL
    };

    struct RHI_Context;

    class RHI_Device
    {
    public:
        RHI_Device() = default;
        virtual ~RHI_Device() = default;

        virtual void Initialize() = 0;

        // Physical Devices
        void RegisterGPU(RHI_GPU&& gpu);
        const RHI_GPU* GetPrimaryGPU();
        void SetPrimaryGPU(const uint32_t index);
        const std::vector<RHI_GPU>& GetGPUs() const { return m_GPUs; }
        
        virtual bool IsInitialized() const = 0;
        std::shared_ptr<RHI_Context>& GetContextRHI() { return m_RHI_Context; }

        // Misc
        static bool IsValidResolution(const uint32_t width, const uint32_t height);
        
        // API
        static std::shared_ptr<RHI_Device> Create();

    protected:
        uint32_t m_CurrentGPUIndex = 0;
        std::vector<RHI_GPU> m_GPUs;
        std::shared_ptr<RHI_Context> m_RHI_Context; // Render contexts are created alongside RHI_Device.
    };
}