#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <memory>
#include "../Source/Vulkan/VulkanDebug.h"
#include "../Source/Vulkan/VulkanDevice.h"
#include "../Source/Vulkan/VulkanSwapchain.h"
#include "../Source/Vulkan/VulkanPipeline.h"
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
		void CreatePresentationSurface();
		void CreateVulkanInstance(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion);

	private:
		VkInstance m_VulkanInstance;
		std::shared_ptr<VulkanDebug> m_DebugMessenger = nullptr;
		std::shared_ptr<VulkanDevice> m_Devices = nullptr;
		std::shared_ptr<VulkanSwapchain> m_Swapchain = nullptr;
		std::shared_ptr<VulkanPipeline> m_Pipeline = nullptr;

		//Presentation
		std::shared_ptr<Window> m_Window = nullptr;
		VkSurfaceKHR m_Surface;

		//Validation
		bool m_ValidationLayersEnabled = false;
	};
}
