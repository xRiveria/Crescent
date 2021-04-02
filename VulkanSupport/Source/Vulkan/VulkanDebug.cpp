#include "VulkanDebug.h"
#include <iostream>

namespace Crescent
{
	//As this is an extension function, it is not automatically loaded. We will thus look up its address ourselves by using vkGetInstanceProcAddr.
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	VulkanDebug::VulkanDebug()
	{

	}

	void VulkanDebug::DestroyDebugInstance()
	{
		DestroyDebugUtilsMessengerEXT(*m_VulkanInstance, m_DebugMessenger, nullptr);
	}

	//Our debug function itself. The VKAPI_ATTR and VKAPI_CALL ensures that the function has the right signature for Vulkan to call it.
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallbackFunction(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		/*
			The first parameter specifies the severity of the message which is one of the following flags:
			- VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic messages.
			- VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the creation of a resource.
			- VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior that may not be an error but very likely a bug in your application.
			- VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes.

			The second parameter specifies a message type filter for the logs you wish to see get printed in addition to the severity level.
			- VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance.
			- VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake.
			- VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan.

			The third parameter refers to a VkDebugUtilsMessengerCallbackDataEXT struct containing the details of the message itself with the most important ones being:
			- pMessage: The debug message as a null terminated string.
			- pObjects: Array of Vulkan object handles related to the message.
			- objectCount: Number of objects in said array.
		*/

		//The values of the enumerations are setup in such a way that we can use a comparison operator to check if a message is equal or worse compared to some level of severity.
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			//Message is important enough to show.
		}

		std::cerr << "Validation Layer: " << pCallbackData->pMessage << "\n";

		//The return value indicates if the Vulkan call that triggered the validation layer message should be aborted. If this returns true, then the call is aborted with the VK_ERROR_VALIDATION_FAILED_EXT error.
		return VK_FALSE;
	}

	bool VulkanDebug::QueryValidationLayersSupport()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> avaliableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, avaliableLayers.data());

		//Check if the validation layers we want are supported.
		for (const char* layerName : m_ValidationLayers)
		{
			bool layerFound = false;
			for (const VkLayerProperties& layerProperties : avaliableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0) //0 is returned if identical.
				{
					layerFound = true;
					std::cout << layerName << " is supported.\n";
					break;
				}
			}

			if (!layerFound)
			{
				std::cout << "Error: " << layerName << " not found.";
				return false;
			}
		}

		return true;
	}

	void VulkanDebug::PopulateDebugMessengerCreationInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo)
	{
		debugMessengerInfo = {};
		debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		/* 
			Allows you to specify all the types of severities you would like your callback to be called for. We've specified all types except
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT and VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT to receive notifications about possible problems 
			while leaving out verbose/general debug information. 
		*/
		debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		
		//Similarly, we can do the same for message types. We've simply enabled all types here.
		debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		
		debugMessengerInfo.pfnUserCallback = DebugCallbackFunction; //Specifies a pointer to the debug function itself. 
		debugMessengerInfo.pUserData = nullptr; //You may pass a pointer to your own data here that will be passed to the debug function.
	}

	void VulkanDebug::SetupDebugMessenger(VkInstance* vulkanInstance)
	{
		m_VulkanInstance = vulkanInstance;

		VkDebugUtilsMessengerCreateInfoEXT creationInfo;
		PopulateDebugMessengerCreationInfo(creationInfo);

		if (CreateDebugUtilsMessengerEXT(*m_VulkanInstance, &creationInfo, nullptr, &m_DebugMessenger))
		{
			throw std::runtime_error("Failed to setup debug messenger.");
		}
		else
		{
			std::cout << "Successfully created Debug Messenger.\n";
		}
	}
}