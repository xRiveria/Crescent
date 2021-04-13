#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanSemaphore
	{
	public:
		VulkanSemaphore(VkDevice* logicalDevice);

	private:
		VkSemaphore m_Semaphore;
		VkDevice* m_LogicalDevice;
	};
}
