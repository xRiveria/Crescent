#pragma once
#include "../Math/Vector4.h"
#include "../RHI_Device.h"
#include "../RHI_Utilities.h"
#include "../API_Utilities/RHI_Display.h"
#include "../API_Utilities/RHI_GPU.h"
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

        inline Vulkan_Context* GetVulkanContext()
        {
            return static_cast<Vulkan_Context*>(GlobalContext::m_RHI_Context); // Static cast here as there is no virtual inheritence in our context.

            // return std::dynamic_pointer_cast<DX11_Context>(rhiContext).get();
        }

        namespace Errors
        {
            inline const char* VulkanErrorToString(const VkResult result)
            {
                switch (result)
                {
                    case VK_SUCCESS:                                            return "VK_SUCCESS";
                    case VK_NOT_READY:                                          return "VK_NOT_READY";
                    case VK_TIMEOUT:                                            return "VK_TIMEOUT";
                    case VK_EVENT_SET:                                          return "VK_EVENT_SET";
                    case VK_EVENT_RESET:                                        return "VK_EVENT_RESET";
                    case VK_INCOMPLETE:                                         return "VK_INCOMPLETE";
                    case VK_ERROR_OUT_OF_HOST_MEMORY:                           return "VK_ERROR_OUT_OF_HOST_MEMORY";
                    case VK_ERROR_OUT_OF_DEVICE_MEMORY:                         return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
                    case VK_ERROR_INITIALIZATION_FAILED:                        return "VK_ERROR_INITIALIZATION_FAILED";
                    case VK_ERROR_DEVICE_LOST:                                  return "VK_ERROR_DEVICE_LOST";
                    case VK_ERROR_MEMORY_MAP_FAILED:                            return "VK_ERROR_MEMORY_MAP_FAILED";
                    case VK_ERROR_LAYER_NOT_PRESENT:                            return "VK_ERROR_LAYER_NOT_PRESENT";
                    case VK_ERROR_EXTENSION_NOT_PRESENT:                        return "VK_ERROR_EXTENSION_NOT_PRESENT";
                    case VK_ERROR_FEATURE_NOT_PRESENT:                          return "VK_ERROR_FEATURE_NOT_PRESENT";
                    case VK_ERROR_INCOMPATIBLE_DRIVER:                          return "VK_ERROR_INCOMPATIBLE_DRIVER";
                    case VK_ERROR_TOO_MANY_OBJECTS:                             return "VK_ERROR_TOO_MANY_OBJECTS";
                    case VK_ERROR_FORMAT_NOT_SUPPORTED:                         return "VK_ERROR_FORMAT_NOT_SUPPORTED";
                    case VK_ERROR_FRAGMENTED_POOL:                              return "VK_ERROR_FRAGMENTED_POOL";
                    case VK_ERROR_OUT_OF_POOL_MEMORY:                           return "VK_ERROR_OUT_OF_POOL_MEMORY";
                    case VK_ERROR_INVALID_EXTERNAL_HANDLE:                      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
                    case VK_ERROR_SURFACE_LOST_KHR:                             return "VK_ERROR_SURFACE_LOST_KHR";
                    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                     return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
                    case VK_SUBOPTIMAL_KHR:                                     return "VK_SUBOPTIMAL_KHR";
                    case VK_ERROR_OUT_OF_DATE_KHR:                              return "VK_ERROR_OUT_OF_DATE_KHR";
                    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                     return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
                    case VK_ERROR_VALIDATION_FAILED_EXT:                        return "VK_ERROR_VALIDATION_FAILED_EXT";
                    case VK_ERROR_INVALID_SHADER_NV:                            return "VK_ERROR_INVALID_SHADER_NV";
                    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
                    case VK_ERROR_FRAGMENTATION_EXT:                            return "VK_ERROR_FRAGMENTATION_EXT";
                    case VK_ERROR_NOT_PERMITTED_EXT:                            return "VK_ERROR_NOT_PERMITTED_EXT";
                    case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:                   return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
                    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:          return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
                    case VK_ERROR_UNKNOWN:                                      return "VK_ERROR_UNKNOWN";
                    case VK_RESULT_MAX_ENUM:                                    return "VK_RESULT_MAX_ENUM";
                }

                return "Unknown Vulkan Error Code.";
            }

            inline bool Check(VkResult result)
            {
                if (result == VK_SUCCESS)
                {
                    return true;
                }

                std::cout << "Vulan Error! " << VulkanErrorToString(result) << "\n";
                return false;
            }
        }

        namespace Layers
        {
            inline bool IsLayerPresent(const char* layerName)
            {
                uint32_t layerCount;
                vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

                std::vector<VkLayerProperties> layers(layerCount);
                vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

                for (const VkLayerProperties& layer : layers)
                {
                    if (strcmp(layerName, layer.layerName) == 0)
                    {
                        std::cout << "Requested layer extension found: " << layerName << "\n";
                        return true;
                    }
                }

                return false;
            }

            inline std::vector<const char*> GetSupportedLayerExtensions(const std::vector<const char*>& layers)
            {
                std::vector<const char*> layersSupported;

                for (const auto& layer : layers)
                {
                    if (IsLayerPresent(layer))
                    {
                        layersSupported.emplace_back(layer);
                    }
                    else
                    {
                        std::cout << "Layer is not supported: " << layer << "\n";
                    }
                }

                return layersSupported;
            }
        }

        namespace Extensions
        {
            inline bool IsDeviceExtensionPresent(const char* extensionName, VkPhysicalDevice physicalDevice)
            {
                uint32_t extensionCount = 0;
                vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

                std::vector<VkExtensionProperties> extensions(extensionCount);
                vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

                for (const VkExtensionProperties& extension : extensions)
                {
                    if (strcmp(extensionName, extension.extensionName) == 0)
                    {
                        std::cout << "Requested device extension found: " << extensionName << "\n";
                        return true;
                    }
                }

                return false;
            }

            inline std::vector<const char*> GetSupportedDeviceExtensions(const std::vector<const char*>& extensions, VkPhysicalDevice physicalDevice)
            {
                std::vector<const char*> extensionsSupported;

                for (const auto& extension : extensions)
                {
                    if (IsDeviceExtensionPresent(extension, physicalDevice))
                    {
                        extensionsSupported.emplace_back(extension);
                    }
                    else
                    {
                        std::cout << "Requested Vulkan extension not found: " << extension << "\n";
                    }
                }

                return extensionsSupported;
            }

            inline bool IsInstanceExtensionPresent(const char* extensionName)
            {
                uint32_t extensionCount = 0;
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

                std::vector<VkExtensionProperties> extensions(extensionCount);
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

                for (const auto& extension : extensions)
                {
                    if (strcmp(extensionName, extension.extensionName) == 0)
                    {
                        std::cout << "Requested instance extension found: " << extensionName << "\n";
                        return true;
                    }
                }

                return false;
            }

            inline std::vector<const char*> GetSupportedInstanceExtensions(const std::vector<const char*>& extensions)
            {
                std::vector<const char*> extensionsSupported;
                
                for (const auto& extension : extensions)
                {
                    if (IsInstanceExtensionPresent(extension))
                    {
                        extensionsSupported.emplace_back(extension);
                    }
                    else
                    {
                        std::cout << "Vulkan instance extension is not supported: " << extension << "\n";
                    }
                }

                return extensionsSupported;
            }
        }

        class Functions
        {
        public:
            Functions() = default;
            ~Functions() = default;

            static void Initialize()
            {
                #define GetFunction(variable, definition)                                                                           \
                variable = reinterpret_cast<PFN_##definition>(vkGetInstanceProcAddr(GetVulkanContext()->m_Instance, #definition));  \
                if (!variable)                                                                                                      \
                {                                                                                                                   \
                    std::cout << "Failed to get function pointer for " << #definition << "\n";                                      \
                }                                                                                                                   \
                else                                                                                                                \
                {                                                                                                                   \
                    std::cout << "Retrieved function pointer for " << #definition << "\n";                                          \
                }                                                                                                                   \

                GetFunction(GetPhysicalDeviceMemoryProperties2, vkGetPhysicalDeviceMemoryProperties2);

                if (GlobalContext::m_RHI_Context->m_DebuggingEnabled)
                {
                    // VK_EXT_debug_utils
                    GetFunction(CreateMessenger, vkCreateDebugUtilsMessengerEXT);
                    GetFunction(DestroyMessenger, vkDestroyDebugUtilsMessengerEXT);
                    GetFunction(MarkerBegin, vkCmdBeginDebugUtilsLabelEXT);
                    GetFunction(MarkerEnd, vkCmdEndDebugUtilsLabelEXT);

                    // VK_EXT_debug_marker
                    GetFunction(SetObjectTag, vkSetDebugUtilsObjectTagEXT);
                    GetFunction(SetObjectName, vkSetDebugUtilsObjectNameEXT);
                }
            }

        public:
            static VkDebugUtilsMessengerEXT m_DebugMessenger;

            // PFN: Pointer to Function
            static PFN_vkCreateDebugUtilsMessengerEXT CreateMessenger;
            static PFN_vkDestroyDebugUtilsMessengerEXT DestroyMessenger;
            static PFN_vkSetDebugUtilsObjectTagEXT SetObjectTag; // Attaches arbitrary data to an object.
            static PFN_vkSetDebugUtilsObjectNameEXT SetObjectName; // Gives a user friendly name to an object.
            static PFN_vkCmdBeginDebugUtilsLabelEXT MarkerBegin; // Opens a command buffer debug label region.
            static PFN_vkCmdEndDebugUtilsLabelEXT MarkerEnd;
            static PFN_vkGetPhysicalDeviceMemoryProperties2KHR GetPhysicalDeviceMemoryProperties2; // Reports memory information for the specified physical device, with extended information in a pNext chain of output structures.
        };

        class Debug
        {
        public:
            Debug() = default;
            ~Debug() = default;

            // Callback function signature.
            static VKAPI_ATTR VkBool32 VKAPI_CALL Callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* userData)
            {
                std::string message = "Vulkan + " + std::string(pCallbackData->pMessage) + "\n";
                
                if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
                {
                    // std::cout << "[Info] " << message;
                }
                else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
                {
                    std::cout << "[Info] " << message;
                }
                else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                {
                    std::cout << "[Warning] " << message;

                }
                else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                {
                    std::cout << "[Error] " << message;
                }

                // The application should always return VK_FALSE. The VK_TRUE value is reserved for use in layer development.
                return VK_FALSE;
            }

            static void Initialize(VkInstance instance)
            {
                if (Functions::CreateMessenger)
                {
                    VkDebugUtilsMessengerCreateInfoEXT creationInfo = {};
                    creationInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                    // Verbose - Diagnostic messages from Vulkan loaders, layers and drivers.
                    // Info - Informational messages such as resource details.
                    // Warning - Specifies use of Vulkan that may expose an application bug. Some cases may not be harmful, although certain behavior are almost certainly bad when unintended such as using an image whose memory has not been filled.
                    // Error - Specifies that the application has violated a valid usage condition of the specification.
                    creationInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                    // General - Specifies that some general event has occured. This is typically a non-specification, non-performance event.
                    // Validation - Specifies that something has occured during validation against the Vulkan specification that may indicate invalid behavior.
                    // Performance - Specifies a potentially non-optimal use of Vulkan, such as using vkCmdClearColorImage when setting VkAttachmentDescription::loadOp where VK_ATTACHMENT_LOAD_OP_CLEAR would have worked.
                    creationInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                    creationInfo.pfnUserCallback = Callback;

                    Functions::CreateMessenger(instance, &creationInfo, nullptr, &Functions::m_DebugMessenger);
                    
                    std::cout << "Successfully setup Vulkan debug messenger.\n";
                }
            }

            static void Shutdown(VkInstance instance)
            {
                if (!Functions::DestroyMessenger)
                {
                    return;
                }

                Functions::DestroyMessenger(instance, Functions::m_DebugMessenger, nullptr);
            }

            static void _SetGPUObjectName(uint64_t object, VkObjectType objectType, const char* name)
            {
                if (!Functions::SetObjectName)
                {
                    return;
                }

                VkDebugUtilsObjectNameInfoEXT nameDescription = {};
                nameDescription.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                nameDescription.pNext = nullptr;
                nameDescription.objectType = objectType; // Type of object, such as VkImage, VkBuffer etc.
                nameDescription.objectHandle = object;
                nameDescription.pObjectName = name;

                Functions::SetObjectName(GetVulkanContext()->m_Device, &nameDescription);
            }

            static void SetGPUObjectTag(uint64_t object, VkObjectType objectType, uint64_t name, size_t tagSize, const void* tag)
            {
                if (!Functions::SetObjectTag)
                {
                    return;
                }

                VkDebugUtilsObjectTagInfoEXT tagDescription = {};
                tagDescription.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
                tagDescription.pNext = nullptr;
                tagDescription.objectType = objectType;
                tagDescription.objectHandle = object;
                tagDescription.tagName = name;
                tagDescription.tagSize = tagSize;
                tagDescription.pTag = tag;

                Functions::SetObjectTag(GetVulkanContext()->m_Device, &tagDescription);
            }

            static void MarkerBegin(VkCommandBuffer commandBuffer, const char* name, const Vector4& color)
            {
                if (!Functions::MarkerBegin)
                {
                    return;
                }

                VkDebugUtilsLabelEXT labelDescription = {};
                labelDescription.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                labelDescription.pNext = nullptr;
                labelDescription.pLabelName = name;
                labelDescription.color[0] = color.x;
                labelDescription.color[1] = color.y;
                labelDescription.color[2] = color.z;
                labelDescription.color[3] = color.w;

                Functions::MarkerBegin(commandBuffer, &labelDescription);
            }

            static void MarkerEnd(VkCommandBuffer commandBuffer)
            {
                if (!Functions::MarkerEnd)
                {
                    return;
                }

                Functions::MarkerEnd(commandBuffer);
            }

            static void SetGPUObjectName(VkCommandPool commandPool, const char* name)
            {
                _SetGPUObjectName((uint64_t)commandPool, VK_OBJECT_TYPE_COMMAND_POOL, name);
            }

            static void SetGPUObjectName(VkCommandBuffer commandBuffer, const char* name)
            {
                _SetGPUObjectName((uint64_t)commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
            }

            static void SetGPUObjectName(VkQueue queue, const char* name)
            {
                _SetGPUObjectName((uint64_t)queue, VK_OBJECT_TYPE_QUEUE, name);
            }

            static void SetGPUObjectName(VkImage image, const char* name)
            {
                _SetGPUObjectName((uint64_t)image, VK_OBJECT_TYPE_IMAGE, name);
            }

            static void SetGPUObjectName(VkImageView imageView, const char* name)
            {
                _SetGPUObjectName((uint64_t)imageView, VK_OBJECT_TYPE_IMAGE_VIEW, name);
            }

            static void SetGPUObjectName(VkSampler sampler, const char* name)
            {
                _SetGPUObjectName((uint64_t)sampler, VK_OBJECT_TYPE_SAMPLER, name);
            }

            static void SetGPUObjectName(VkBuffer buffer, const char* name)
            {
                _SetGPUObjectName((uint64_t)buffer, VK_OBJECT_TYPE_BUFFER, name);
            }

            static void SetGPUObjectName(VkBufferView bufferView, const char* name)
            {
                _SetGPUObjectName((uint64_t)bufferView, VK_OBJECT_TYPE_BUFFER_VIEW, name);
            }

            static void SetGPUObjectName(VkDeviceMemory memory, const char* name)
            {
                _SetGPUObjectName((uint64_t)memory, VK_OBJECT_TYPE_DEVICE_MEMORY, name);
            }

            static void SetGPUObjectName(VkShaderModule shaderModule, const char* name)
            {
                _SetGPUObjectName((uint64_t)shaderModule, VK_OBJECT_TYPE_SHADER_MODULE, name);
            }

            static void SetGPUObjectName(VkPipeline pipeline, const char* name)
            {
                _SetGPUObjectName((uint64_t)pipeline, VK_OBJECT_TYPE_PIPELINE, name);
            }

            static void SetGPUObjectName(VkPipelineLayout pipelineLayout, const char* name)
            {
                _SetGPUObjectName((uint64_t)pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, name);
            }

            static void SetGPUObjectName(VkRenderPass renderPass, const char* name)
            {
                _SetGPUObjectName((uint64_t)renderPass, VK_OBJECT_TYPE_RENDER_PASS, name);
            }

            static void SetGPUObjectName(VkFramebuffer framebuffer, const char* name)
            {
                _SetGPUObjectName((uint64_t)framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, name);
            }

            static void SetGPUObjectName(VkDescriptorSetLayout descriptorSetLayout, const char* name)
            {
                _SetGPUObjectName((uint64_t)descriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, name);
            }

            static void SetGPUObjectName(VkDescriptorSet descriptorSet, const char* name)
            {
                _SetGPUObjectName((uint64_t)descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, name);
            }

            static void SetGPUObjectName(VkDescriptorPool descriptorPool, const char* name)
            {
                _SetGPUObjectName((uint64_t)descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, name);
            }

            static void SetGPUObjectName(VkSemaphore semaphore, const char* name)
            {
                _SetGPUObjectName((uint64_t)semaphore, VK_OBJECT_TYPE_SEMAPHORE, name);
            }

            static void SetGPUObjectName(VkFence fence, const char* name)
            {
                _SetGPUObjectName((uint64_t)fence, VK_OBJECT_TYPE_FENCE, name);
            }

            static void SetGPUObjectName(VkEvent event, const char* name)
            {
                _SetGPUObjectName((uint64_t)event, VK_OBJECT_TYPE_EVENT, name);
            }
        };

        namespace DevicesAndQueues
        {
            inline uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags, const std::vector<VkQueueFamilyProperties>& queueFamilyProperties, uint32_t* index)
            {
                // Dedicated queue for compute for not graphics.
                if (queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
                    {
                        if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
                        {
                            *index = i;
                            return true;
                        }
                    }
                }

                // Dedicated queue for transfer. Try to find a queue family index that supports transfer but not graphics and compute.
                if (queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
                    {
                        if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
                            && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
                        {
                            *index = i;
                            return true;
                        }
                    }
                }

                // For other queue types or if no seperate compute queue is present, return the first one to support the requested flags. 
                for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
                {
                    if (queueFamilyProperties[i].queueFlags & queueFlags)
                    {
                        *index = i;
                        return true;
                    }
                }

                return false;
            }

            inline bool GetQueueFamilyIndices(const VkPhysicalDevice& physicalDevice)
            {
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

                std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

                if (!GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, queueFamilyProperties, &GetVulkanContext()->m_QueueGraphicsIndex))
                {
                    std::cout << "Graphics queue not supported.\n";
                    return false;
                }

                if (!GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, queueFamilyProperties, &GetVulkanContext()->m_QueueTransferIndex))
                {
                    std::cout << "Transfer queue not supported, using Graphics queue instead.\n";
                    return false;
                }

                if (!GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, queueFamilyProperties, &GetVulkanContext()->m_QueueComputeIndex))
                {
                    std::cout << "Compute queue not supported, using Graphics queue instead.\n";
                    return false;
                }

                return true;
            }

            inline bool ChoosePhysicalDevice()
            {
                // Register all physical devices.
                uint32_t deviceCount = 0;
                if (!Errors::Check(vkEnumeratePhysicalDevices(GetVulkanContext()->m_Instance, &deviceCount, nullptr)))
                {
                    return false;
                }

                if (deviceCount == 0)
                {
                    std::cout << "There are no avaliable devices.\n";
                    return false;
                }

                std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
                if (!Errors::Check(vkEnumeratePhysicalDevices(GetVulkanContext()->m_Instance, &deviceCount, physicalDevices.data())))
                {
                    return false;
                }

                // Go through all the devices.
                for (const VkPhysicalDevice& physicalDevice : physicalDevices)
                {
                    // Get device properties.
                    VkPhysicalDeviceProperties deviceProperties = {};
                    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

                    VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};
                    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

                    RHI_GPU_Type deviceType = RHI_GPU_Type::Unknown;
                    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                    {
                        deviceType = RHI_GPU_Type::Integrated;
                    }
                    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                    {
                        deviceType = RHI_GPU_Type::Discrete;
                    }
                    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
                    {
                        deviceType = RHI_GPU_Type::Virtual;
                    }
                    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
                    {
                        deviceType = RHI_GPU_Type::CPU;
                    }

                    // Let the engine know about it as it will sort all the devices from best to worst automatically in RegisterGPU().
                    GlobalContext::m_RHI_Device->RegisterGPU(RHI_GPU(
                        // deviceProperties.apiVersion,                                     // API Version
                        // deviceProperties.driverVersion,                                  // Driver Version
                        deviceProperties.vendorID,                                          // Vendor ID
                        deviceType,                                                         // Type
                        &deviceProperties.deviceName[0],                                    // Name
                        static_cast<uint64_t>(deviceMemoryProperties.memoryHeaps[0].size),  // Memory
                        static_cast<void*>(physicalDevice)                                  // Data
                    ));
                }

                // Go through all the devices (sorted from best to worse based on their properties).
                for (uint32_t deviceIndex = 0; deviceIndex < static_cast<uint32_t>(GlobalContext::m_RHI_Device->GetGPUs().size()); deviceIndex++)
                {
                    const RHI_GPU& gpu = GlobalContext::m_RHI_Device->GetGPUs()[deviceIndex];
                    VkPhysicalDevice vulkanPhysicalDevice = static_cast<VkPhysicalDevice>(gpu.GetInternalData());

                    // Get the first device that has a graphics, a compute and a transfer queue.
                    if (GetQueueFamilyIndices(vulkanPhysicalDevice))
                    {
                        GlobalContext::m_RHI_Device->SetPrimaryGPU(deviceIndex);
                        GetVulkanContext()->m_PhysicalDevice = vulkanPhysicalDevice;
                        break;
                    }
                }

                return true;
            }
        }

        namespace Display
        {
            inline void DetectDisplayModes()
            {
                const bool updateFPSLimitToHighestHertz = true;

                RHI_Display::RegisterDisplayMode(RHI_DisplayMode(640, 480, 165, 1), updateFPSLimitToHighestHertz);
                RHI_Display::RegisterDisplayMode(RHI_DisplayMode(720, 576, 165, 1), updateFPSLimitToHighestHertz);
                RHI_Display::RegisterDisplayMode(RHI_DisplayMode(1280, 720, 165, 1), updateFPSLimitToHighestHertz);
                RHI_Display::RegisterDisplayMode(RHI_DisplayMode(1920, 1080, 165, 1), updateFPSLimitToHighestHertz);
                RHI_Display::RegisterDisplayMode(RHI_DisplayMode(2560, 1440, 165, 1), updateFPSLimitToHighestHertz);
            }
        }

        namespace CommandPool
        {
            inline bool Create(void*& commandPool, const RHI_Queue_Type queueType)
            {
                VkCommandPoolCreateInfo commandPoolInfo = {};
                commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                commandPoolInfo.queueFamilyIndex = GlobalContext::m_RHI_Context->GetQueueIndex(queueType);
                commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allows any command buffer allocated from the pool to be individually reset to its initial state, either by calling vkResetCommandBuffer or implict reset through vkBeginCommandBuffer.
                
                VkCommandPool* vulkanCommandPool = reinterpret_cast<VkCommandPool*>(&commandPool);
                return Errors::Check(vkCreateCommandPool(GetVulkanContext()->m_Device, &commandPoolInfo, nullptr, vulkanCommandPool));
            }

            inline void Destroy(void*& commandPool)
            {
                VkCommandPool vulkanCommandPool = static_cast<VkCommandPool>(commandPool);
                vkDestroyCommandPool(GetVulkanContext()->m_Device, vulkanCommandPool, nullptr);
                commandPool = nullptr;
            }
        }
    }
}