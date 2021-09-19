#pragma once
#include <string>
#include "RHI_Context.h"

namespace Aurora
{
    enum RHI_Format
    {
        RHI_Format_R8G8B8A8_Unorm
    };

    inline std::string RenderAPIToString(RenderAPI renderAPI)
    {
        switch (renderAPI)
        {
            case RenderAPI::Vulkan:
                return "Vulkan";

            case RenderAPI::DirectX11:
                return "DirectX 11";

            case RenderAPI::DirectX12:
                return "DirectX 12";

            case RenderAPI::OpenGL:
                return "OpenGL";

            case RenderAPI::Unknown:
                return "Unknown";
        }

        return "Unknown";
    }
}