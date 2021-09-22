#include "RHI_PCH.h"
#include "Vulkan_Utilities.h"

namespace Aurora::Vulkan_Utilities
{
    VkDebugUtilsMessengerEXT Functions::m_DebugMessenger = nullptr;
    PFN_vkCreateDebugUtilsMessengerEXT Functions::CreateMessenger = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT Functions::DestroyMessenger = nullptr;
    PFN_vkSetDebugUtilsObjectTagEXT Functions::SetObjectTag = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT Functions::SetObjectName = nullptr;
    PFN_vkCmdBeginDebugUtilsLabelEXT Functions::MarkerBegin = nullptr;
    PFN_vkCmdEndDebugUtilsLabelEXT Functions::MarkerEnd = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties2KHR Functions::GetPhysicalDeviceMemoryProperties2 = nullptr;
}