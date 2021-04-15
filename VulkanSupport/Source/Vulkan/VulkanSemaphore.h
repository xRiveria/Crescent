#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanSemaphore
	{
	public:
		VulkanSemaphore();

		void CreateVulkanSemaphore(VkDevice* logicalDevice);
		VkSemaphore* RetrieveSemaphore() { return &m_Semaphore; }

	private:
		VkSemaphore m_Semaphore;
		VkDevice* m_LogicalDevice;
	};
}
