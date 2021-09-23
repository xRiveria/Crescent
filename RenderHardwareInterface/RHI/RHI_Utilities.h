#pragma once
#include <string>
#include "RHI_Device.h"

namespace Aurora
{
    enum RHI_Format : uint32_t
    {
        RHI_Format_Undefined,
        RHI_Format_R8G8B8A8_Unorm,

        // RGB
        RHI_Format_R11G11B10_Float,

        // Depth
        RHI_Format_D32_Float,
        RHI_Format_D32_Float_S8X24_UInt,
    };

    enum RHI_Present_Mode : uint32_t
    {
        RHI_Present_Immediate    = 1 << 0,  // Doesn't wait. Frames are not dropped. Tearing.
        RHI_Present_Mailbox      = 1 << 1,  // Waits for V-Blank. Frames are dropped. No tearing.
        RHI_Present_FIFO         = 1 << 2,  // Waits for V-Blank always. Frames are not dropped. No tearing.
        RHI_Present_FIFO_Relaxed = 1 << 3,  // Waits for V-Blank once. Frames are not dropped. Tearing.

        // D3D11 Only Flags
        RHI_Swap_Discard            = 1 << 4,
        RHI_Swap_Sequential         = 1 << 5,   // Backbuffer contents are persisted. Contents of the swapchain are presented in order from the first buffer (buffer 0) to the last buffer.
        RHI_Swap_Flip_Sequential    = 1 << 6,   // Backbuffer contents are persisted.
        RHI_Swap_Flip_Discard       = 1 << 7,   // Backbuffer contents are discarded.
        RHI_Swap_Allow_Mode_Switch  = 1 << 8
    };
    
    enum RHI_Queue_Type
    {
        RHI_Queue_Graphics,
        RHI_Queue_Compute,
        RHI_Queue_Transfer
    };

    // DX12
    enum RHI_Descriptor_Heap_Type
    {
        RHI_Descriptor_Heap_Type_CBV_SRV_UAV,
        RHI_Descriptor_Heap_Type_Sampler,
        RHI_Descriptor_Heap_Type_RTV,
        RHI_Descriptor_Heap_Type_DSV
    };

    // DX12
    enum RHI_Descriptor_Heap_Flag
    {
        RHI_Descriptor_Heap_Flag_None,
        RHI_Descriptor_Heap_Flag_ShaderVisible
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