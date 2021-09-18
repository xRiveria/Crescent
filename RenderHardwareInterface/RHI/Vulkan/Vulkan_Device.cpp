#include "Vulkan_Device.h"
#include "Vulkan_Context.h"
#include "Vulkan_Utilities.h"
#include <iostream>

namespace Aurora
{
    void Vulkan_Device::Initialize()
    {
        m_RHI_Context = std::make_shared<Vulkan_Context>();

        // Pass our pointers into the global utility namespace.
        Vulkan_Utilities::GlobalContext::m_RHI_Context = m_RHI_Context.get();
        Vulkan_Utilities::GlobalContext::m_RHI_Device = this;

        std::cout << "Successfully initialized Vulkan.\n";
        m_IsInitialized = true;
    }
}