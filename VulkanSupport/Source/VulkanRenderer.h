#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <memory>
#include "../Source/Vulkan/VulkanDebug.h"
#include "../Source/Vulkan/VulkanDevice.h"
#include "../Source/Vulkan/VulkanSwapchain.h"
#include "../Source/Vulkan/VulkanDescriptorLayout.h"
#include "../Source/Vulkan/VulkanPipeline.h"
#include "../Source/Vulkan/VulkanCommandPool.h"
#include "../Source/Vulkan/VulkanResource.h"
#
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
		std::shared_ptr<VulkanDescriptorLayout> m_DescriptorLayout = nullptr;
		std::shared_ptr<VulkanPipeline> m_Pipeline = nullptr;
		std::shared_ptr<VulkanCommandPool> m_CommandPool = nullptr;

		//Presentation
		std::shared_ptr<Window> m_Window = nullptr;
		VkSurfaceKHR m_Surface;

		//Validation
		bool m_ValidationLayersEnabled = false;

		//Custom
		std::shared_ptr<VulkanTexture> m_ModelTexture = nullptr;
		std::shared_ptr<VulkanResource> m_Model = nullptr;
	};
}
