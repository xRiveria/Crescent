#include "VulkanFence.h"
#include <stdexcept>
#include <iostream>

namespace Crescent
{
	VulkanFence::VulkanFence(VkDevice* logicalDevice) : m_LogicalDevice(logicalDevice)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(*logicalDevice, &fenceInfo, nullptr, &m_Fence) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Fence.\n");
		}
		else
		{
			std::cout << "Successfully created Fence.\n";
		}
	}
}