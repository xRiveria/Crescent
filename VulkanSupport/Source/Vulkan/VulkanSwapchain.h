#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VkDevice* logicalDevice);

	private:
		void CreateSwapchain();

	private:
		VkSwapchainKHR m_Swapchain;
		VkDevice* m_LogicalDevice;
	};
}