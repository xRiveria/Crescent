#include "VulkanIndexBuffer.h"

namespace Crescent
{
	VulkanIndexBuffer::VulkanIndexBuffer(std::shared_ptr<VulkanResource> resource, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkQueue* graphicsQueue, VkCommandPool* commandPool)
		: m_Resource(resource), m_PhysicalDevice(physicalDevice), m_LogicalDevice(logicalDevice), m_GraphicsQueue(graphicsQueue), m_CommandPool(commandPool)
	{
		CreateIndexBuffer();
	}

	void VulkanIndexBuffer::DestroyIndexBufferInstance()
	{
		vkDestroyBuffer(*m_LogicalDevice, m_IndexBuffer, nullptr);
		vkFreeMemory(*m_LogicalDevice, m_IndexBufferMemory, nullptr);
	}

	void VulkanIndexBuffer::CreateIndexBuffer()
	{
		/*
			There are only 2 notable differences between the creation of the Index and Vertex buffer. The bufferSize is now equal to the number of indices times the size of the 
			indes type, either uint16_t or uint32_t. The usage of the indexBuffer should be VK_BUFFER_USAGE_INDEX_BUFFER_BIT instead of VK_BUFFER_USAGE_VERTEX_BUFFER_BIT. We 
			will create a staging buffer to copty the contents of the indices to and then copy it to the final device local index buffer.
		*/
		VkDeviceSize bufferSize = sizeof(m_Resource->m_Indices[0]) * m_Resource->m_Indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		//Note that VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT specifies that memory allocated with this type can be mapped for host access using vkMapMemory.
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, *m_LogicalDevice, *m_PhysicalDevice);

		void* data;
		vkMapMemory(*m_LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m_Resource->m_Indices.data(), (size_t)bufferSize);
		vkUnmapMemory(*m_LogicalDevice, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory, *m_LogicalDevice, *m_PhysicalDevice);
		CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize, *m_CommandPool, *m_LogicalDevice, *m_GraphicsQueue);

		vkDestroyBuffer(*m_LogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*m_LogicalDevice, stagingBufferMemory, nullptr);

		/*
			We have previously mentioned that we should allocate multiple resources lke buffers from a single memory allocation. However, we should go a step further. Driver
			developers recommend that we should also store multiple buffers like the vertex and index buffer into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffer. 
			The advantage is that your data is more cache friendly in that case because its closer together. It is even possible to reuse the same chunk of memory for multiple resources 
			if they are not used during the same render operations, provided their data is refreshed of course. This is known as aliasing and some Vulkan functions have explicit 
			flags to specify that you want to do this.
		*/

		std::cout << "Successfully created Index Buffer.\n";
	}
}