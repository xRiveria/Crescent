#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace Crescent
{
	class VulkanDebug
	{
	public:
		VulkanDebug();
		void DestroyDebugInstance();

		bool QueryValidationLayersSupport();
		void PopulateDebugMessengerCreationInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo);
		void SetupDebugMessenger(VkInstance* vulkanInstance);

		VkDebugUtilsMessengerEXT* RetrieveDebugMessenger() { return &m_DebugMessenger; }

	public:
		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

	private:		
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkInstance* m_VulkanInstance = nullptr;
	};
}
