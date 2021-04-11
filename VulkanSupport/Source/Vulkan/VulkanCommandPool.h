#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanCommandPool
	{
	public:
		VulkanCommandPool(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkSurfaceKHR* presentationSurface);
		void DestroyCommandPoolInstance();

		VkCommandPool* RetrieveCommandPool() { return &m_CommandPool; }

	private:
		void CreateCommandPool();

	private:
		VkCommandPool m_CommandPool;

		VkSurfaceKHR* m_PresentationSurface;
		VkPhysicalDevice* m_PhysicalDevice;
		VkDevice* m_LogicalDevice;
	};
}