#include "VulkanSemaphore.h"
#include <stdexcept>
#include <iostream>

namespace Crescent
{
	VulkanSemaphore::VulkanSemaphore()
	{

	}

	void VulkanSemaphore::CreateVulkanSemaphore(VkDevice* logicalDevice)
	{
		m_LogicalDevice = logicalDevice;
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(*logicalDevice, &semaphoreInfo, nullptr, &m_Semaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Semaphore.\n");
		}
		else
		{
			std::cout << "Successfully created Semaphore.\n";
		}
	}
}