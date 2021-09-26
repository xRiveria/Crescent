#pragma once
#include <string>
#include "../Math/Vector4.h"
#include "RHI_Device.h"

namespace Aurora
{
    static const Vector4  m_RHI_Color_Load = Vector4(std::numeric_limits<float>::infinity(), 0.0f, 0.0f, 0.0f);
    static const float    m_RHI_Depth_Load = std::numeric_limits<float>::infinity();
    static const uint32_t m_RHI_Stencil_Load = (std::numeric_limits<uint32_t>::max)() - 1;

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

    enum RHI_Cull_Mode
    {
        RHI_Cull_None,
        RHI_Cull_Front,
        RHI_Cull_Back,
    };

    enum RHI_Fill_Mode
    {
        RHI_Fill_Solid,
        RHI_Fill_Wireframe,
    };

    enum RHI_Comparison_Function
    {
        RHI_Comparison_Function_Never,                // The comparison will never pass.
        RHI_Comparison_Function_Less,                 // If the source data is less than the destination data, the comparison passes.
        RHI_Comparison_Function_Equal,                // If the source data is equal to the destination data, the comparison passes.
        RHI_Comparison_Function_Less_Equal,           // If the source data is lesser than or equal to the destination data, the comparison passes.
        RHI_Comparison_Function_Greater,              // If the source data is greater than the destination data, the comparison passes.
        RHI_Comparison_Function_Not_Equal,            // If the source data is not equal to the destination data, the comparison passes.
        RHI_Comparison_Function_Greater_Equal,        // If the source data is greater than or equal to the destination data, the comparison passes.
        RHI_Comparison_Function_Always                // Always passes the comparison.
    };

    enum RHI_Stencil_Operation
    {
        RHI_Stencil_Operation_Keep,             // Keeps the existing stencil data.
        RHI_Stencil_Operation_Zero,             // Sets the stencil data to 0.
        RHI_Stencil_Operation_Replace,          // Sets the stencil data to the reference value set through OMSetStencilRef.
        RHI_Stencil_Operation_Increment_Sat,    // Increment the stencil buffer entry by 1, and clamps the result to the maximum value of our stencil buffer (eg 255 for 8-bit stencil buffers).
        RHI_Stencil_Operation_Decrement_Sat,    // Same as the above, but for decrementations.
        RHI_Stencil_Operation_Invert,           // Inverts the bits of our stencil data.
        RHI_Stencil_Operation_Increment,        // Increment the stencil buffer entry by 1. If the incremented value exceeds our stencil buffer limits, wrap it to 0.
        RHI_Stencil_Operation_Decrement         // Decrement the stencil buffer entry by 1. If the decremented value is less than 0, wrap it to the maximum allowed value.
    };

    // Operation between two pixel values to be blended after blending functions are applied.
    enum RHI_Blend_Operation
    {
        RHI_Blend_Operation_Add,                // Adds source 1 and 2.
        RHI_Blend_Operation_Subtract,           // Subtracts the source pixel from the target pixel.
        RHI_Blend_Operation_Reverse_Subtract,   // Subtracts the target pixel from the source pixel.
        RHI_Blend_Operation_Minimum,            // Finds the minimum of source 1 and source 2.
        RHI_Blend_Operation_Maximum             // Finds the maximum of source 1 and source 2.
    };

    // Function to apply to the current pixel value before blend operations are applied.
    enum RHI_Blend_Function
    {
        RHI_Blend_Function_Zero,                          // Generates RGBA * (0, 0, 0, 0).
        RHI_Blend_Function_One,                           // Generates RGBA * (1, 1, 1, 1).
        RHI_Blend_Function_Source_Color,                  // Generates RGBA * RGB.
        RHI_Blend_Function_Inverse_Source_Color,          // Generates RGBA * (1 - RGB).
        RHI_Blend_Function_Source_Alpha,                  // Generates RGBA * Source Alpha Value.
        RHI_Blend_Function_Inverse_Source_Alpha,          // Generates RGBA * (1 - Source Alpha Value).
        RHI_Blend_Function_Destination_Alpha,             // Generates RGBA * (Render Target (Destination) Alpha)
        RHI_Blend_Function_Inverse_Destination_Color,     // Generates RGBA * (1 - Render Target (Destination) Alpha)
        RHI_Blend_Function_Source_Alpha_Sat,              // Generates RGBA * (F, F, F, 1), where F = min(A, 1 - A). 
        RHI_Blend_Function_Blend_Factor,                  // Generates RGBA * Blend Factor (Set with OMSetBlendFactor in DX)
        RHI_Blend_Function_Inverse_Blend_Factor,          // Generates RGBA * 1 - Blend Factor (Set with OMSetBlendFactor in DX)
        RHI_Blend_Function_Source1_Color,
        RHI_Blend_Function_Inverse_Source1_Color,
        RHI_Blend_Function_Source1_Alpha,
        RHI_Blend_Function_Inverse_Source1_Alpha
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