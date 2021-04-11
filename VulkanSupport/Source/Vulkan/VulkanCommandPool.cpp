#include "VulkanCommandPool.h"
#include "VulkanUtilities.h"

namespace Crescent
{
	VulkanCommandPool::VulkanCommandPool(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkSurfaceKHR* presentationSurface) 
		: m_LogicalDevice(logicalDevice), m_PhysicalDevice(physicalDevice), m_PresentationSurface(presentationSurface)
	{
		CreateCommandPool();
	}

	void VulkanCommandPool::DestroyCommandPoolInstance()
	{
		vkDestroyCommandPool(*m_LogicalDevice, m_CommandPool, nullptr);
	}

	void VulkanCommandPool::CreateCommandPool()
	{
		/*
			Commands in Vulkan, like drawing operations and memory transfers are not executed directly from function calls. You have to record all of the operations you want to
			perform in command buffer objects. The advantage of this is that all of the hard work of setting up drawing commands are done in advance and in multiple threads. After
			that, you just have to tell Vulkan to execute the commands in the main loop.

			We have to create a command pool before we can create command buffers. Command pools manage the memory that is used to store the buffers and command buffers are thus
			allocated from them. Command buffers are executed by submitting them on one of the device's queues, like the graphics and presentation queues we had retrieved. Each
			command pool can only allocate command buffers that are submitted on a single type of queue. We're going to record commands for drawing, which is why we have chosen
			the graphics queue family.

			There are 2 possible flags for command pools:
			- VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior).
			- VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag, they will all have to be reset together.
			We will only record the command buffers at the beginning of the porgram and then execute them many times in the main loop, thus, we don't have to use any of these flags.
		*/
		QueueFamilyIndices queueFamilies = QueryQueueFamilySupport(*m_PhysicalDevice, *m_PresentationSurface);

		VkCommandPoolCreateInfo poolCreationInfo{};
		poolCreationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolCreationInfo.queueFamilyIndex = queueFamilies.m_GraphicsFamily.value();
		poolCreationInfo.flags = 0;

		if (vkCreateCommandPool(*m_LogicalDevice, &poolCreationInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Command Pool.\n");
		}
		else
		{
			std::cout << "Successfully created Command Pool.\n";
		}
	}
}
