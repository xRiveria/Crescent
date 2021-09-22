#include "RHI_PCH.h"
#include "Vulkan_Device.h"
#include "Vulkan_Context.h"
#include "Vulkan_Utilities.h"

namespace Aurora
{
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

        applicationDescription.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationDescription.pApplicationName = "Application Name";
        applicationDescription.applicationVersion = 1;
        applicationDescription.pEngineName = "Engine Name";
        applicationDescription.engineVersion = 1;
        applicationDescription.apiVersion = Vulkan_Utilities::GetVulkanContext()->m_APIVersion; // Must be the highest version of Vulkan the application is designed to use.

        // Get the supported extensions out of the requested extensions.
        std::vector<const char*> extensionsSupported = Vulkan_Utilities::Extensions::GetSupportedInstanceExtensions(Vulkan_Utilities::GetVulkanContext()->m_ExtensionsInstance);

        VkInstanceCreateInfo instanceDescription = {};
        instanceDescription.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceDescription.pApplicationInfo = &applicationDescription;
        instanceDescription.enabledExtensionCount = static_cast<uint32_t>(extensionsSupported.size());
        instanceDescription.ppEnabledExtensionNames = extensionsSupported.data();
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

        // Find a suitable physical device.


        std::cout << "Successfully initialized Vulkan.\n";
        m_IsInitialized = true;
    }
}