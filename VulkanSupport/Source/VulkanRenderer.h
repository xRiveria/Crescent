#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Crescent
{
	class VulkanRenderer
	{
	public:
		VulkanRenderer(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion, const bool& validationLayersEnabled);

		void DrawFrames();

	private:
		void CreateVulkanInstance(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion);

	private:
		VkInstance m_VulkanInstance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;

		//Validation
		bool m_ValidationLayersEnabled = false;
		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	};
}
