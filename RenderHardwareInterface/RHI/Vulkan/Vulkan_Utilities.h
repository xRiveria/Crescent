#pragma once
#include "../RHI_Device.h"
#include "Vulkan_Context.h"

namespace Aurora
{
    namespace Vulkan_Utilities
    {
        namespace GlobalContext
        {
            static inline RHI_Device* m_RHI_Device = nullptr;
            static inline RHI_Context* m_RHI_Context = nullptr;
        };
    }
}