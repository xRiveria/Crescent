#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <memory>
#include "../Source/Vulkan/VulkanDebug.h"
#include "Window.h"

namespace Crescent
{
	class VulkanRenderer
	{
	public:
		VulkanRenderer(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion, const bool& validationLayersEnabled);
		~VulkanRenderer();

		void DrawFrames();

	private:
		void CreateVulkanInstance(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion);

	private:
		std::shared_ptr<Window> m_Window = nullptr;
		VkInstance m_VulkanInstance;
		std::shared_ptr<VulkanDebug> m_DebugMessenger = nullptr;

		//Validation
		bool m_ValidationLayersEnabled = false;
	};
}
