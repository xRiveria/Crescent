#include "RHI_PCH.h"
#include "RHI_SwapChain.h"
#include "DX11/DX11_SwapChain.h"
#include "DX12/DX12_SwapChain.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    std::shared_ptr<RHI_SwapChain> RHI_SwapChain::Create(uint32_t width, uint32_t height, void* windowHandle, RHI_Device* rhiDevice, uint32_t bufferCount, uint32_t flags, RHI_Format format, const char* objectName)
    {
        switch (Renderer::GetCurrentRenderAPI())
        {
            case RenderAPI::DirectX11: return std::make_shared<DX11_SwapChain>(width, height, windowHandle, rhiDevice, bufferCount, flags, format, objectName);
            case RenderAPI::DirectX12: return std::make_shared<DX12_SwapChain>(width, height, windowHandle, rhiDevice, bufferCount, flags, format, objectName);
            // case RenderAPI::Vulkan:    return std::make_shared<Vulkan_Device>();
            // So on and so forth...
        }

        std::cout << "Unknown Render API. SwapChain could not be created.\n";
        return nullptr;
    }
}