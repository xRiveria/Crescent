#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <stdexcept>
#include <iostream>

namespace Crescent
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR m_Capabilities; //Minimum or maximum number of images in the swapchain, the minimum/maximum width and height of the images etc.
		std::vector<VkSurfaceFormatKHR> m_Formats; //Pixel Format/Color Space.
		std::vector<VkPresentModeKHR> m_PresentationModes; //Avaliable presentation modes.
	};

	struct QueueFamilyIndices
	{
		/*
			We use optional here to indicate whether a particular queue family was found. This is good as while we may "prefer" devices with a dedicated transfer queue family,
			we may not actually require it. It is likely that the queues themselves end up being from the same queue family, but throughout our program, we will treat them
			as thoughg they were seperate queues for a uniform approach. Nevertheless, we could add logic that explicitly prefers a physical device that supports
			drawing and presentation in the same queue for improved performance.
		*/
		std::optional<uint32_t> m_GraphicsFamily;
		std::optional<uint32_t> m_PresentationFamily; //While Vulkan supports WSI (Windows System Integration), it doesn't mean that every device will support it.

		bool IsComplete()
		{
			return m_GraphicsFamily.has_value() && m_PresentationFamily.has_value();
		}
	};

	static QueueFamilyIndices QueryQueueFamilySupport(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& presentationSurface)
	{
		/*
			Almost every operation in Vulkan from drawing to texture uploads require commands to be submitted to a queue. Thre are different queues that a device may support,
			such as a graphics rendering queue, texture uploading queue etc. In Vulkan, queues are collected into queue families. These queue families each contain
			a sedt of queues which all support the same type of operations. We thus need to check which queue families are supported by the device and which ones
			support the commands that we want to use.
		*/

		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			VkBool32 presentationSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, presentationSurface, &presentationSupport);

			if (presentationSupport) //If any supports presentation queues...
			{
				indices.m_PresentationFamily = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) //If any supports graphics queues...
			{
				indices.m_GraphicsFamily = i;
			}

			if (indices.IsComplete()) //If all conditions were already fulfilled, exit the loop.
			{
				break;
			}

			i++;
		}

		return indices;
	}

	static SwapchainSupportDetails QuerySwapchainSupport(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& presentationSurface)
	{
		/*
			Vulkan does not have the concept of a default framebuffer. Hence, it requires an infrastructure that will own the buffers we will render to before we visualize
			them on the screen. This infrastructure is known as the swapchain and must be created explictly in Vulkan. The swapchain is essentially a queue of images that
			are waiting to be presented to the screen. Our application will acquire such an image to draw to it and then return it to the queue. How exactly the queue
			works and the conditions for presenting an image from the queue depends on how the swapchain is setup, but the general purpose of the swapchain is to synchronize
			the presentation of images with the refresh rate of the screen (how many times the screen is updated with new frames per second).
		*/
		SwapchainSupportDetails swapchainDetails;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, presentationSurface, &swapchainDetails.m_Capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentationSurface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			swapchainDetails.m_Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentationSurface, &formatCount, swapchainDetails.m_Formats.data());
		}

		uint32_t presentationModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentationSurface, &presentationModeCount, nullptr);
		if (presentationModeCount != 0)
		{
			swapchainDetails.m_PresentationModes.resize(presentationModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentationSurface, &presentationModeCount, swapchainDetails.m_PresentationModes.data());
		}

		return swapchainDetails;
	}

	static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, const VkPhysicalDevice& physicalDevice)
	{
		/*
			Graphic cards can offer different types of memory to allocate from. Each type of memory varies in terms of allowed operations and performance characteristics.
			We need to combine the requirements of the buffer and our own application requirements to find the right type of memory to use.
		*/
		VkPhysicalDeviceMemoryProperties memoryProperties;
		/*
			The VkPhysicalDeviceMemoryProperties structure has 2 arrays: memoryTypes and memoryHeaps. Memory heaps are distinct memory resources like dedicated VRAM and
			swap space in RAM for when VRAM runs out. The different types of memory exist within these heaps. Right now, we will only concern ourselves with the type of memory and
			not the heap it comes from, but you can imagine that this can affect performance.
		*/
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			/*
				The typeFilter parameter will be used to specify the bit field of memory types that are suitable. That means that we can find the index of a suitable memory
				type by simply iterating over them and checking if the corresponding bit is set to 1. However, we're not just interested in a memory type that is suitable
				for our buffer. We also need to be able to write our data to that memory. The memoryTypes array consists of VkMemoryType structs that specify the 
				heap and properties of each type of memory. The properties define special features of the memory, like being able to map it so we can write to it from the CPU.
				This property is indicated with VK_MEMORY_HOST_VISIBLE_BIT, but we also need to use the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT property. We will see why when we map the memory.
				
				We may have more than one desirable property, so we should check if the result of the bitwise AND is not just non-zero, but also equal to the desired properties bit field.
				If there is a memory type suitable for the buffer that also has all the properties we need, then return its index. Otherwise, we throw an exception.
			*/
			if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		//If we can't find the right memory type...
		throw std::runtime_error("Failed to find a suitable memory type.");
	}

	static void CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, const VkDevice& logicalDevice, const VkPhysicalDevice& physicalDevice)
	{
		/*
			Buffers in Vulkan are regions of memory for storing arbitrary data that can be read by the graphics card. They can be used to store vertex data, but also have many
			other purposes. Unlike the standard Vulkan objects that we have been dealing with so far, buffers do not automatically allocate memory for themselves. Like many things
			in Vulkan, the API puts the programmer in control of almost everything and memory management is one of those things.
		*/
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = bufferSize; //Refers to the size of the buffer in bytes. This is very straightforward with sizeof().
		bufferInfo.usage = bufferUsage; //Indicates for which purposes the data in the buffer is going to be used. It is possible to specify multiple purposes using a bitwise or operator.
		//Just like images in the swapchain, buffers can be owned by a specific queue family or be shared between multiple at the same time. As this buffer will only be used from the
		//graphics queue, we can stick with exclusive access.
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//There is a flags parameter that is used to configure sparse buffer memory, which is not relevant at the moment. We will leave it at a default value of 0.

		if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Buffer.");
		}
		else
		{
			std::cout << "Successfully created Buffer.\n";
		}

		/*
			While the buffer has been created, it doesn't actually have any memory assigned to it yet. The first step of allocating memory for the buffer is to query its memory
			requirements using the aptly named vkGetBufferMemoryRequirements function.
		*/
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(logicalDevice, buffer, &memoryRequirements); //Retrieve memory required by the created buffer.
		/*
			The VkMemoryRequirements struct has 3 fields:
			- size: The size of the required amount of memory in bytes. This may differ from bufferInfo.size.
			- alignment: The offset in bytes where the buffer begins in the allocated region of memory. This depends on bufferInfo.usage and bufferInfo.flags.
			- memoryTypeBits: Bit field of the memory types that are suitable for the buffer.

			Memory allocation is now as simple as specifying the size and type, both of which are derived from the memory requirements of the buffer and the desired properties.
			It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory for every individual buffer. The maximum number
			of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an
			NVIDEA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create a custom allocator that splits up a single allocation
			among many different objects using the offset parameters that we have seen in many functions. We can either implement such an allocator ourselves, or use the
			VulkanMemoryAllocator library. However, it is okay for us to use a seperate allocation for every resource for now as we won't come close to hitting of those limits.
		*/
		VkMemoryAllocateInfo memoryAllocationInfo{};
		memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocationInfo.allocationSize = memoryRequirements.size;
		memoryAllocationInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties, physicalDevice);

		if (vkAllocateMemory(logicalDevice, &memoryAllocationInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory.");
		}
		else
		{
			std::cout << "Successfully allocated buffer memory.\n";
		}

		/*
			Once successful, we can associate this memory with the buffer directly. The first three parameters are straightforward, and the fourth parameter is the offset
			within the region of memory. Since this memory is allocated specifically for this: the buffer we are creating, the offset is simply 0. If the offset is non-zero,
			then it is required to be divisible by memoryRequirements.alignment.
		*/
		vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
	}
}