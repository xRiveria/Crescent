#pragma once
#include <vulkan/vulkan.h>
#include "VulkanResource.h"
#include <memory>

namespace Crescent
{
	class VulkanIndexBuffer
	{
	public:
		VulkanIndexBuffer(std::shared_ptr<VulkanResource> resource, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkQueue* graphicsQueue, VkCommandPool* commandPool);
		void DestroyIndexBufferInstance();
		VkBuffer* RetrieveIndexBuffer() { return &m_IndexBuffer; }

	private:
		void CreateIndexBuffer();

	private:
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		std::shared_ptr<VulkanResource> m_Resource;
		VkCommandPool* m_CommandPool;
		VkQueue* m_GraphicsQueue;
		VkPhysicalDevice* m_PhysicalDevice;
		VkDevice* m_LogicalDevice;
	};
}
