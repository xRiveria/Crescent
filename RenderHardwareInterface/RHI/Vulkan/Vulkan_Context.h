#pragma once
#include "../RHI_Context.h"

#pragma comment(lib, "vulkan-1.lib")
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Aurora
{
    struct Vulkan_Context : public RHI_Context
    {
        VkInstance m_Instance = nullptr;
        VkPhysicalDevice m_PhysicalDevice = nullptr;
        VkDevice m_Device = nullptr;
        VkPhysicalDeviceProperties m_DeviceProperties = {};
    };
}