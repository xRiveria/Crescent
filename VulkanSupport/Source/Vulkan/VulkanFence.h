#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanFence
	{
	public:
		VulkanFence();

		void CreateVulkanFence(VkDevice *logicalDevice);
		//VkFence* RetrieveVulkanFence() { return &m_Fence; }

	public:
		VkFence m_Fence;

	private:
		VkDevice* m_LogicalDevice;
	};
}
