#include "VulkanRenderer.h"
#include <stdexcept>
#include <iostream>
#include "../Source/Vulkan/VulkanDebug.h"

namespace Crescent
{
	VulkanRenderer::VulkanRenderer(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion, const bool& validationLayersEnabled)
		: m_ValidationLayersEnabled(validationLayersEnabled)
	{
		CreateVulkanInstance(applicationName, engineName, applicationMainVersion, applicationSubVersion);
		if (validationLayersEnabled)
		{
			m_DebugMessenger = VulkanDebug::SetupDebugMessenger(m_VulkanInstance);
		}
	}

	//Retrieves the names of extensions required by our Vulkan application.
	std::vector<const char*> RetrieveRequiredVulkanExtensions(const bool& validationLayersEnabled)
	{
		//Vulkan is platform agnostic. Thus, extensions are needed to interface with the window system. GLFW has a handy function that returns the extension(s) it needs to do that.
		std::vector<const char*> extensions;
		
		uint32_t glfwExtensionsCount = 0;
		const char** glfwExtensions;
		//Retrieve GLFW stuff. How do we do it nicely?

		if (validationLayersEnabled)
		{
			/*
				While certain extensions (such as the GLFW ones) are always required, the debug messenger extension is conditionally added. Validation layers will allow Vulkan
				to print debug messages to the standard output by default, which we can overwrite with our own explict callback. This will allow us to decide what kind of
				messages we want to see as not all of them are necessarily important/fatal enough.

				VK_EXT_debug_utils will allow us to setup a debug messenger to handle debug messages and their associated details.
			*/
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); //Equals the literal string of "VK_EXT_debug_utils".
		}

		return extensions;
	}

	void VulkanRenderer::CreateVulkanInstance(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion)
	{
		//Validation layers are optional components that hook into Vulkan function calls to apply operations such as parameter checking, Vulkan object tracking, call logging etc.
		if (m_ValidationLayersEnabled && !VulkanDebug::QueryValidationLayersSupport(m_ValidationLayers))
		{
			throw::std::runtime_error("Validation Layers Requested, but not avaliable!");
		}

		//Passes information on our application to the implementation.
		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = applicationName.c_str();
		applicationInfo.applicationVersion = VK_MAKE_VERSION(applicationMainVersion, applicationSubVersion, 0);
		applicationInfo.pEngineName = engineName.c_str();
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion = VK_API_VERSION_1_0;

		//Passes information on which global extensions and validation layers we wish to use, as well as our application information to the Vulkan drivers.
		VkInstanceCreateInfo creationInfo{};
		creationInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		creationInfo.pApplicationInfo = &applicationInfo;

		//Extensions Information.
		std::vector<const char*> requiredExtensions = RetrieveRequiredVulkanExtensions(m_ValidationLayersEnabled);
		creationInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		creationInfo.ppEnabledExtensionNames = requiredExtensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreationInfo; //This is placed outside the if statement so that it is not destroyed before the vkCreateInstance call below.
		if (m_ValidationLayersEnabled)
		{
			creationInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			creationInfo.ppEnabledLayerNames = m_ValidationLayers.data(); //Determines the global validation layers based on our specifications.
			/*
				Our debugger is usually created after the instance is created. Thus, we populate the pNext struct member with our debug messenger creation so it will be used 
				automatically during vkCreateInstance and cleaned up after vkDestroyInstance.
			*/
			VulkanDebug::PopulateDebugMessengerCreationInfo(debugCreationInfo);
			creationInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreationInfo;
		}
		else
		{
			creationInfo.enabledLayerCount = 0;
			creationInfo.pNext = nullptr;
		}
		
		if (vkCreateInstance(&creationInfo, nullptr, &m_VulkanInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan instance.");
		}
		else
		{
			std::cout << "Successfully created Vulkan Instance.\n";
		}
	}

	void VulkanRenderer::DrawFrames()
	{

	}
}