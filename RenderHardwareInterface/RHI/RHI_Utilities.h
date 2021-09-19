#pragma once
#include <string>
#include "RHI_Device.h"
#include "RHI_Context.h"

namespace Aurora
{
    enum RHI_Format
    {
        RHI_Format_R8G8B8A8_Unorm
    };

    enum RHI_Present_Mode : uint32_t
    {
        RHI_Present_Immediate    = 1 << 0,  // Doesn't wait. Frames are not drooped. Tearing.
        RHI_Present_Mailbox      = 1 << 1,  // Waits for V-Blank. Frames are dropped. No tearing.
        RHI_Present_FIFO         = 1 << 2,  // Waits for V-Blank always. Frames are not dropped. No tearing.
        RHI_Present_FIFO_Relaxed = 1 << 3,  // Waits for V-Blank once. Frames are not dropped. Tearing.

        // D3D11 Only Flags
        RHI_Swap_Discard            = 1 << 4,
        RHI_Swap_Sequential         = 1 << 5,
        RHI_Swap_Flip_Sequential    = 1 << 6,
        RHI_Swap_Flip_Discard       = 1 << 7,
        RHI_Swap_Allow_Mode_Switch  = 1 << 8
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