#include <iostream>
#include "RHI_Device.h"
#include "DX11/DX11_Device.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    std::shared_ptr<RHI_Device> RHI_Device::Create()
    {
        switch (Renderer::GetCurrentRenderAPI())
        {
            case RenderAPI::DirectX11: return std::make_shared<DX11_Device>();
            case RenderAPI::DirectX12: return std::make_shared<DX11_Device>();
            // So on and so forth...
        }

        std::cout << "Unknown Render API. Context could not be created.";
        return nullptr;
    }
}