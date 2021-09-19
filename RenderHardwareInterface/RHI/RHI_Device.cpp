#include <iostream>
#include <algorithm>
#include "RHI_Device.h"
#include "DX11/DX11_Device.h"
#include "DX12/DX12_Device.h"
#include "Vulkan/Vulkan_Device.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    void RHI_Device::RegisterGPU(RHI_GPU&& gpu)
    {
        m_GPUs.emplace_back(std::move(gpu));
        std::cout << "Registered GPU: " << m_GPUs.back().GetGPUName() << ", " << m_GPUs.back().GetGPUMemory() << "\n";

        // Sort in descending order based on highest memory avaliable.
        std::sort(m_GPUs.begin(), m_GPUs.end(), [](const RHI_GPU& firstGPU, const RHI_GPU& secondGPU)
        {
            return firstGPU.GetGPUMemory() > secondGPU.GetGPUMemory();
        });
    }

    const RHI_GPU* RHI_Device::GetPrimaryGPU()
    {
        if (m_CurrentGPUIndex >= m_GPUs.size())
        {
            return nullptr;
        }

        return &m_GPUs[m_CurrentGPUIndex];
    }

    void RHI_Device::SetPrimaryGPU(const uint32_t index)
    {
        m_CurrentGPUIndex = index;

        if (const RHI_GPU* gpu = GetPrimaryGPU())
        {
            std::cout << "Set GPU As: " << gpu->GetGPUName() << ", " << gpu->GetGPUMemory() << "\n";
        }
    }

    std::shared_ptr<RHI_Device> RHI_Device::Create()
    {
        switch (Renderer::GetCurrentRenderAPI())
        {
            case RenderAPI::DirectX11: return std::make_shared<DX11_Device>();
            case RenderAPI::DirectX12: return std::make_shared<DX12_Device>();
            case RenderAPI::Vulkan:    return std::make_shared<Vulkan_Device>();
            // So on and so forth...
        }

        std::cout << "Unknown Render API. Context could not be created.\n";
        return nullptr;
    }
}