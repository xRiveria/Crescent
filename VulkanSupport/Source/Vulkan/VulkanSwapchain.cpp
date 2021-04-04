#include "VulkanSwapchain.h"
#include "VulkanUtilities.h"

namespace Crescent
{
	VulkanSwapchain::VulkanSwapchain(VkDevice* logicalDevice) : m_LogicalDevice(logicalDevice)
	{
		CreateSwapchain();
	}

	void VulkanSwapchain::CreateSwapchain()
	{

	}
}