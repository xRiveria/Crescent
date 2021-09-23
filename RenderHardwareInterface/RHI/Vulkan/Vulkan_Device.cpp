#include "RHI_PCH.h"
#include "Vulkan_Device.h"
#include "Vulkan_Context.h"
#include "Vulkan_Utilities.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    Vulkan_Device::~Vulkan_Device()
    {
        if (!m_RHI_Context || !m_RHI_Context->m_QueueGraphics)
        {
            return;
        }

        // Command Pool Destruction
        Vulkan_Utilities::CommandPool::Destroy(m_CommandPool);

        // Release Resources
        if (m_RHI_Context->m_DebuggingEnabled)
        {
            Vulkan_Utilities::Debug::Shutdown(Vulkan_Utilities::GetVulkanContext()->m_Instance);
        }

        vkDestroyDevice(Vulkan_Utilities::GetVulkanContext()->m_Device, nullptr);
        vkDestroyInstance(Vulkan_Utilities::GetVulkanContext()->m_Instance, nullptr);
    }

    void Vulkan_Device::Initialize()
    {
        m_RHI_Context = std::make_shared<Vulkan_Context>();

        // Pass our pointers into the global utility namespace.
        Vulkan_Utilities::GlobalContext::m_RHI_Context = m_RHI_Context.get();
        Vulkan_Utilities::GlobalContext::m_RHI_Device = this;

        // Create Instance
        VkApplicationInfo applicationDescription = {};
        
        // Deduce API version to use. Get SDK version. Get driver version.
        uint32_t sdkVersion = VK_HEADER_VERSION_COMPLETE; // Complete version number of the vulkan_core.h header.
        uint32_t driverVersion = 0;
        {
            // Per LunarG, if vkEnumerateInstanceVersion is not present, we are running on Vulkan 1.0: https://www.lunarg.com/wp-content/uploads/2019/02/Vulkan-1.1-Compatibility-Statement_01_19.pdf
            PFN_vkEnumerateInstanceVersion eiv = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
            if (eiv)
            {
                eiv(&driverVersion);
            }
            else
            {
                driverVersion = VK_API_VERSION_1_0;
            }
        }

        // Choose the version that is supported by both the SDK and the driver.
        Vulkan_Utilities::GetVulkanContext()->m_APIVersion = Math::Utilities::Min(sdkVersion, driverVersion);

        // If the SDK isn't supported by the driver, prompt the user to update.
        if (sdkVersion > driverVersion)
        {

        }

        applicationDescription.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationDescription.pApplicationName = "Application Name";
        applicationDescription.applicationVersion = 1;
        applicationDescription.pEngineName = "Engine Name";
        applicationDescription.engineVersion = 1;
        applicationDescription.apiVersion = Vulkan_Utilities::GetVulkanContext()->m_APIVersion; // Must be the highest version of Vulkan the application is designed to use.

        // Get the supported extensions out of the requested extensions.
        std::vector<const char*> instanceExtensionsSupported = Vulkan_Utilities::Extensions::GetSupportedInstanceExtensions(Vulkan_Utilities::GetVulkanContext()->m_ExtensionsInstance);

        VkInstanceCreateInfo instanceDescription = {};
        instanceDescription.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceDescription.pApplicationInfo = &applicationDescription;
        instanceDescription.enabledExtensionCount = static_cast<uint32_t>(instanceExtensionsSupported.size());
        instanceDescription.ppEnabledExtensionNames = instanceExtensionsSupported.data();
        instanceDescription.enabledLayerCount = 0;
        
        // Validation features.
        VkValidationFeaturesEXT validationFeaturesDescription = {};
        validationFeaturesDescription.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validationFeaturesDescription.enabledValidationFeatureCount = static_cast<uint32_t>(Vulkan_Utilities::GetVulkanContext()->m_ValidationExtensions.size());
        validationFeaturesDescription.pEnabledValidationFeatures = Vulkan_Utilities::GetVulkanContext()->m_ValidationExtensions.data();

        // We will only enable all debug/validation features in our Debug build.
        if (Vulkan_Utilities::GetVulkanContext()->m_DebuggingEnabled)
        {
            // Enable validation layer.
            if (Vulkan_Utilities::Layers::IsLayerPresent(Vulkan_Utilities::GetVulkanContext()->m_ValidationLayers.front()))
            {
                instanceDescription.enabledLayerCount = static_cast<uint32_t>(Vulkan_Utilities::GetVulkanContext()->m_ValidationLayers.size());
                instanceDescription.ppEnabledLayerNames = Vulkan_Utilities::GetVulkanContext()->m_ValidationLayers.data();
                instanceDescription.pNext = &validationFeaturesDescription; // Capture events that occur when creating or destroying an instance. Valid only for the duration of the vkCreateInstance and vkDestroyInstance calls. Persistent callback objects are to be created seperately with vkCreateDebugUtilsMessengertEXT.
            }
            else
            {
                std::cout << "The Vulkan Validation Layer was requested, but could not be found.\n";
            }
        }

        if (!Vulkan_Utilities::Errors::Check(vkCreateInstance(&instanceDescription, nullptr, &Vulkan_Utilities::GetVulkanContext()->m_Instance)))
        {
            return;
        }

        std::cout << "Successfully created Vulkan Instance.\n";

        // Get function pointers from extensions.
        Vulkan_Utilities::Functions::Initialize();

        // Debug
        if (Vulkan_Utilities::GetVulkanContext()->m_DebuggingEnabled)
        {
            Vulkan_Utilities::Debug::Initialize(Vulkan_Utilities::GetVulkanContext()->m_Instance);
        }

        // Find a suitable physical device and queries for device queues - Compute, Graphics and Transfer.
        if (!Vulkan_Utilities::DevicesAndQueues::ChoosePhysicalDevice())
        {
            std::cout << "Failed to get a suitable physical device.\n";
            return;
        }

        // Device - Queue Create Info
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::vector<uint32_t> uniqueQueueFamilies =
        {
            m_RHI_Context->m_QueueGraphicsIndex,
            m_RHI_Context->m_QueueTransferIndex,
            m_RHI_Context->m_QueueComputeIndex
        };

        // Each queue is assigned a priority. The priority of each queue is a normalized floating point value between 0.0 and 1.0.
        // This is translated to a discrete priority level by the implementation. Naturally, high values indicate higher priority, which will affect processing time allocation.
        // There is no guarentee with regards to ordering or scheduling among queues with the same priority, other than constraints defined by explicit synchronization primitives. 
        float queuePriority = 1.0f;

        for (const uint32_t& queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily; // Index of the queue family in which to create the queue on this device.
            queueCreateInfo.queueCount = 1; 
            queueCreateInfo.pQueuePriorities = &queuePriority; 

            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Get device properties.
        vkGetPhysicalDeviceProperties(static_cast<VkPhysicalDevice>(Vulkan_Utilities::GetVulkanContext()->m_PhysicalDevice), &Vulkan_Utilities::GetVulkanContext()->m_DeviceProperties);

        // Get resource limtis.
        m_RHI_Context->m_Texture2D_Dimensions_Max = Vulkan_Utilities::GetVulkanContext()->m_DeviceProperties.limits.maxImageDimension2D;
        
        if (m_RHI_Context->m_ProfilingEnabled && !Vulkan_Utilities::GetVulkanContext()->m_DeviceProperties.limits.timestampComputeAndGraphics)
        {
            std::cout << "[Warning] Device does not support timestamps. Disabling profiler...\n";
            m_RHI_Context->m_ProfilingEnabled = false;
        }

        // Get device features.
        VkPhysicalDeviceVulkan12Features deviceFeatures1_2Enabled = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        VkPhysicalDeviceFeatures2 deviceFeaturesEnabled = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &deviceFeatures1_2Enabled }; // pNext to our extension.
        
        // Macro
        #define ENABLE_FEATURE(deviceFeatures, enabledFeatures, feature)                                             \
        if (deviceFeatures.feature)                                                                                  \
        {                                                                                                            \
            std::cout << "Requested device feature " << #feature << " found and enabled. \n";                        \
            enabledFeatures.feature = VK_TRUE;                                                                       \
        }                                                                                                            \
        else                                                                                                         \
        {                                                                                                            \
            std::cout << "Requested device feature " << #feature << " is not supported by the physical device. \n";  \
            enabledFeatures.feature = VK_FALSE;                                                                      \
        } 

        // Get Features
        vkGetPhysicalDeviceFeatures2(Vulkan_Utilities::GetVulkanContext()->m_PhysicalDevice, &Vulkan_Utilities::GetVulkanContext()->m_DeviceFeatures);

        // Enable
        ENABLE_FEATURE(Vulkan_Utilities::GetVulkanContext()->m_DeviceFeatures.features, deviceFeaturesEnabled.features, samplerAnisotropy);
        ENABLE_FEATURE(Vulkan_Utilities::GetVulkanContext()->m_DeviceFeatures.features, deviceFeaturesEnabled.features, fillModeNonSolid);
        ENABLE_FEATURE(Vulkan_Utilities::GetVulkanContext()->m_DeviceFeatures.features, deviceFeaturesEnabled.features, wideLines);
        ENABLE_FEATURE(Vulkan_Utilities::GetVulkanContext()->m_DeviceFeatures.features, deviceFeaturesEnabled.features, imageCubeArray);
        ENABLE_FEATURE(Vulkan_Utilities::GetVulkanContext()->m_DeviceFeatures.features, deviceFeaturesEnabled.features, tessellationShader);
        ENABLE_FEATURE(Vulkan_Utilities::GetVulkanContext()->m_DeviceFeatures.features, deviceFeaturesEnabled.features, geometryShader);
        ENABLE_FEATURE(Vulkan_Utilities::GetVulkanContext()->m_DeviceFeature1_2, deviceFeatures1_2Enabled, timelineSemaphore);

        // Determine enabled graphics shader stages.
        m_EnabledGraphicsShaderStages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        if (deviceFeaturesEnabled.features.geometryShader)
        {
            m_EnabledGraphicsShaderStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
        }
        
        if (deviceFeaturesEnabled.features.tessellationShader)
        {
            m_EnabledGraphicsShaderStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
        }
        
        /*      
            - Enable partially bound descriptors: http://roar11.com/2019/06/vulkan-textures-unbound/ 

            If our descriptor set layout declares that it uses 1000 descriptors and we only load the first 100, the validation layer will see the 
            uninitialized contents after our first 100 textures and think that we intend to index them in the shader, which will be an error. 
            Enabling "descriptorBindingPartiallyBound" allows for true support for partial binding.
        */
        VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {};
        descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        descriptorIndexingFeatures.descriptorBindingPartiallyBound = true;
        descriptorIndexingFeatures.runtimeDescriptorArray = true; // Support for unbounded arrays of texture.
        deviceFeaturesEnabled.pNext = &descriptorIndexingFeatures;
        std::cout << "Enabled support for runtimeDescriptorArray and descriptorBindingPartiallyBound.\n";

        // Get the supported extensions out of our requested extensions.
        std::vector<const char*> deviceExtensionsSupported = Vulkan_Utilities::Extensions::GetSupportedDeviceExtensions(Vulkan_Utilities::GetVulkanContext()->m_ExtensionsDevice, Vulkan_Utilities::GetVulkanContext()->m_PhysicalDevice);

        // Device creation.
        VkDeviceCreateInfo deviceCreationInfo = {};
        deviceCreationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreationInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreationInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreationInfo.pNext = &deviceFeaturesEnabled;
        deviceCreationInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsSupported.size());
        deviceCreationInfo.ppEnabledExtensionNames = deviceExtensionsSupported.data();

        if (m_RHI_Context->m_DebuggingEnabled)
        {
            deviceCreationInfo.enabledLayerCount = static_cast<uint32_t>(Vulkan_Utilities::GetVulkanContext()->m_ValidationLayers.size());
            deviceCreationInfo.ppEnabledLayerNames = Vulkan_Utilities::GetVulkanContext()->m_ValidationLayers.data();
        }
        else
        {
            deviceCreationInfo.enabledLayerCount = 0;
        }

        // Device Creation
        if (!Vulkan_Utilities::Errors::Check(vkCreateDevice(Vulkan_Utilities::GetVulkanContext()->m_PhysicalDevice, &deviceCreationInfo, nullptr, &Vulkan_Utilities::GetVulkanContext()->m_Device)))
        {
            std::cout << "Failed to create Vulkan Device.\n";
            return;
        }

        std::cout << "Successfully created Vulkan Device.\n";
        
        // Get queues created alongside our fresh device. Since we only created 1 of each queue, we will retrieve index 0 for each queue type.
        vkGetDeviceQueue(Vulkan_Utilities::GetVulkanContext()->m_Device, m_RHI_Context->m_QueueGraphicsIndex, 0, reinterpret_cast<VkQueue*>(&m_RHI_Context->m_QueueGraphics));
        vkGetDeviceQueue(Vulkan_Utilities::GetVulkanContext()->m_Device, m_RHI_Context->m_QueueComputeIndex, 0, reinterpret_cast<VkQueue*>(&m_RHI_Context->m_QueueCompute));
        vkGetDeviceQueue(Vulkan_Utilities::GetVulkanContext()->m_Device, m_RHI_Context->m_QueueTransferIndex, 0, reinterpret_cast<VkQueue*>(&m_RHI_Context->m_QueueTransfer));

        // Display Modes
        Vulkan_Utilities::Display::DetectDisplayModes();

        // Create command pool.
        Vulkan_Utilities::CommandPool::Create(m_CommandPool, RHI_Queue_Type::RHI_Queue_Graphics);

        /// Detect and log Vulkan version + register as library.

        std::cout << "Successfully initialized Vulkan.\n";
        m_IsInitialized = true;
    }
} 