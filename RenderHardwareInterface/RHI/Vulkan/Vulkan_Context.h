#pragma once
#include "../RHI_Context.h"

#pragma comment(lib, "vulkan-1.lib")
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Aurora
{
    struct Vulkan_Context : public RHI_Context
    {
        uint32_t m_APIVersion = 0;

        VkInstance m_Instance = nullptr;
        VkPhysicalDevice m_PhysicalDevice = nullptr;
        VkDevice m_Device = nullptr;
        VkPhysicalDeviceProperties m_DeviceProperties = {};

#ifdef _DEBUG
        /*
            - VK_LAYER_KHRONOS_validation: The main comprehensive Khronos validation layer. This layer encompasses the entire functionality of the layers listed 
            below, and supercedes them. As the other layers are deprecated, we should use this for all validation going forward. Validation layers are crucial 
            for checking parameter misuse, tracking creation and destruction of objects to detect resource leaks, checking thread safety, logging and tracking Vulkan 
            calls for profiling and replaying.

            - VK_EXT_debug_utils: Create a debug messenger nwhich will pass along debug messages to an application supplied callback. Identify specific Vulkan 
            objects using a name or tag to improve tracking. Identify specific sections within a VkQueue or VkCommandBuffer using labels to aid organization 
            and offline analysis in external tools.
        */

        // Physical Device Extensions (VkDevice)
        std::vector<const char*> m_ExtensionsDevice = { "VK_KHR_swapchain", "VK_EXT_memory_budget", "VK_EXT_depth_clip_enable", "VK_KHR_timeline_semaphore" };
        // Validation Layers (Debugging)
        std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
        // Validation Layer Extensions (Debugging)
        std::vector<VkValidationFeatureEnableEXT> m_ValidationExtensions = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT, VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT };
        // Logical Device Extensions (VkInstance)
        // - VK_EXT_debug_utils allows an application to register multiple callbacks with any Vulkan component wishing to report debug information - file logging, debug breaks or whatsoever application defined behavior.
        std::vector<const char*> m_ExtensionsInstance = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report", "VK_EXT_debug_utils" }; 
        #else
        std::vector<const char*> m_ExtensionsDevice = { "VK_KHR_swapchain", "VK_EXT_memory_budget", "VK_EXT_depth_clip_enable", "VK_KHR_timeline_semaphore" };
        std::vector<const char*> m_ValidationLayers = { };
        std::vector<VkValidationFeatureEnableEXT> m_ValidationExtensions = { };
        std::vector<const char*> m_ExtensionsInstance = { "VK_KHR_surface", "VK_KHR_win32_surface" };
        #endif
    };
}