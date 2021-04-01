#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace Crescent
{
	class VulkanDebug
	{
	public:
		static bool QueryValidationLayersSupport(const std::vector<const char*> validationLayers);
		static void PopulateDebugMessengerCreationInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo);
		static VkDebugUtilsMessengerEXT SetupDebugMessenger(const VkInstance& vulkanInstance);
	};
}
