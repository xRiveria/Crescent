#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanFence
	{
	public:
		VulkanFence(VkDevice *logicalDevice);

	private:
		VkFence m_Fence;
		VkDevice* m_LogicalDevice;
	};
}
