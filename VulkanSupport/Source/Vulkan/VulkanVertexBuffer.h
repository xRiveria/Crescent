#pragma once
#include <vulkan/vulkan.h>
#include "VulkanResource.h"
#include <memory>

namespace Crescent
{
	class VulkanVertexBuffer
	{
	public:
		VulkanVertexBuffer(std::shared_ptr<VulkanResource> resource, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkQueue* graphicsQueue, VkCommandPool* commandPool);

		VkBuffer* RetrieveVertexBuffer() { return &m_VertexBuffer; }

	private:
		void CreateVertexBuffer();

	private:
		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		std::shared_ptr<VulkanResource> m_Resource;
		VkCommandPool* m_CommandPool;
		VkQueue* m_GraphicsQueue;
		VkPhysicalDevice* m_PhysicalDevice;
		VkDevice* m_LogicalDevice;
	};
}