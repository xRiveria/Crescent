#include "VulkanVertexBuffer.h"

namespace Crescent
{
	VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<VulkanResource> resource, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkQueue* graphicsQueue, VkCommandPool* commandPool)
		: m_Resource(resource), m_PhysicalDevice(physicalDevice), m_LogicalDevice(logicalDevice), m_GraphicsQueue(graphicsQueue), m_CommandPool(commandPool)
	{
		CreateVertexBuffer();
	}

	void VulkanVertexBuffer::DestroyVertexBufferInstance()
	{
		vkDestroyBuffer(*m_LogicalDevice, m_VertexBuffer, nullptr);
		vkFreeMemory(*m_LogicalDevice, m_VertexBufferMemory, nullptr);
	}

	void VulkanVertexBuffer::CreateVertexBuffer()
	{
		/*
			While using the vertex buffer directly works just fine, the memory type that allows us to access it from the CPU may not be the most optimal type for the
			graphic card itself to read from. The most optimal memory has the VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT flag and is usually not accessible by the CPU on dedicated 
			graphic cards. We will thus create 2 vertex buffers. One staging buffer in CPU accessible memory to upload the data from the vertex array to, and the final 
			vertex buffer in device local memory. We will then use a buffer copy command to move the data from the staging buffer to the actual vertex buffer.

			We will now be using a stagingBuffer with stagingBufferMemory for mapping and copying the vertex data. We have two new buffer flags here to use:

			- VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation.
			- VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation.

			With this, the vertexBuffer is now allocated from a memory type that is device local, which generally means that we're not able to use vkMapMemory. However, we can 
			copy data from the stagingBuffer to our vertexBuffer. We have to indicate that we intend to do that by specifying the transfer source flag for the stagingBuffer 
			and the destination flag for the vertexBuffer, along with the vertex buffer usage flag. 
		*/
		VkDeviceSize bufferSize = sizeof(m_Resource->m_Vertices[0]) * m_Resource->m_Vertices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, *m_LogicalDevice, *m_PhysicalDevice);
	
		//It is now time to copy the vertex data to the buffer. This is done by mapping the buffer memory into CPU accessible memory with VkMapMemory.
		void* data; //Pointer to the mapped memory.
		
		/*
			This function allows us to access a region of the specified memory resource defined by an offset and size. The offset and size here are 0 and bufferSize respectively. 
			It is also possible to specify the special value VK_WHOLE_SIZE to map all of the memory. The second to last parameter can be used to specify flags, but there aren't any 
			yet avaliable in the current API. It must be set to the value 0. The last parameter specifies the output for the pointer to the mapped memory.
		*/
		vkMapMemory(*m_LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);

		/*
			We can now simply memcpy the vertex data to the mapped memory and unmap it again using vkUnmapMemory. Unfortunately, the driver may not immediately copy the data 
			into the buffer memory, for example because of caching. It is also possible that writes to the buffer are not visible in the mapped memory yet. There are two ways 
			to deal with that problem:

			- Use a memory heap that is coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.
			- Call vkFlushMappedMemoryRanges after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges before reading to the mapped memory.

			We went for the first approach, which ensures that the mapped memory always matches the contents of the allocated memory. Do keep in mind that this may lead to 
			slightly worse performance than explicit flushing, but we will see why that doesn't matter eventually.

			Flushing memory ranges or using a coherent memory heap means that the driver will be aware of our writes to the buffer, but it doesn't actually mean that they are 
			visible to the GPU yet. The transfer of data to the GPU is an operation that happens in the background and the specification simply tells us that it is guarenteed to 
			be complete as of the next call to vkQueueSubmit.
		*/
		memcpy(data, m_Resource->m_Vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(*m_LogicalDevice, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory, *m_LogicalDevice, *m_PhysicalDevice);
		CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize, *m_CommandPool, *m_LogicalDevice, *m_GraphicsQueue);

		vkDestroyBuffer(*m_LogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*m_LogicalDevice, stagingBufferMemory, nullptr);

		std::cout << "Successfully created Vertex Buffer.\n";
	}
}