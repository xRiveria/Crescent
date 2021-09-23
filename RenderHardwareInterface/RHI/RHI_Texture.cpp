#include "RHI_PCH.h"
#include "RHI_Texture2D.h"
#include "../Renderer/Renderer.h"
#include "DX11/DX11_Texture.h"
#include "DX12/DX12_Texture.h"

namespace Aurora
{
    std::shared_ptr<RHI_Texture> RHI_Texture::Create(const uint32_t flags, RHI_Texture_Type textureType, const char* objectName)
    {
        switch (Renderer::GetCurrentRenderAPI())
        {
        case RenderAPI::DirectX11: return std::make_shared<DX11_Texture>(flags, textureType, objectName);
            // case RenderAPI::DirectX12: return std::make_shared<DX12_Device>();
            // case RenderAPI::Vulkan:    return std::make_shared<Vulkan_Device>();
            // So on and so forth...
        }

        std::cout << "Unknown Render API. SwapChain could not be created.\n";
        return nullptr;
    }

    std::shared_ptr<RHI_Texture> RHI_Texture::Create(const uint32_t width, const uint32_t height, const uint32_t mipCount, const RHI_Format format, const uint32_t flags, RHI_Texture_Type textureType, const char* objectName)
    {
        switch (Renderer::GetCurrentRenderAPI())
        {
            case RenderAPI::DirectX11: return std::make_shared<DX11_Texture>(width, height, mipCount, format, flags, textureType, objectName);
            case RenderAPI::DirectX12: return std::make_shared<DX12_Texture>(width, height, mipCount, format, flags, textureType, objectName);
                // case RenderAPI::Vulkan:    return std::make_shared<Vulkan_Device>();
                // So on and so forth...
        }

        std::cout << "Unknown Render API. SwapChain could not be created.\n";
        return nullptr;
    }
}