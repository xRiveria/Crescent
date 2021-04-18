#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <array>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace Crescent
{
	struct Vertex
	{
		glm::vec3 m_Position;
		glm::vec3 m_Color;
		glm::vec2 m_TexCoord;

		//The next step is to tell Vulkan how to pass this data format to the vertex shader once its been uploaded into GPU memory. There are two types of structures
		//we can use to convey this information. The first one is VkVertexInputBindingDescription.
		static VkVertexInputBindingDescription RetrieveBindingDescription()
		{
			/*
				A vertex binding describes at which rate to load data from memory throughout the vertices. It specifies the number of bytes between data entries and whether to move
				to the next data entry after each vertex or after each instance.

				All of our per-vertex data is packed together in one array, so we only have 1 binding. The binding parameter specifies the index of the binding in the array of bindings.
				The stride parameter specifies the number of bytes from one entry to the next, and the inputRate parameter can have one of the following values:
				- VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex.
				- VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance.
			*/
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		//Allows us to do comparisons for our user defined type.
		bool operator==(const Vertex& other) const
		{
			return m_Position == other.m_Position && m_Color == other.m_Color && m_TexCoord == other.m_TexCoord;
		}

		/*
			The second structure that describes how to handle vertex input is VkVertexInputAttributeDescription. We will use a helper function to help populate the struct.
			We will add a VkVertexInputAttributeDescription struct here for our texture coordinates. This is so we can use it to access texture coordinates as input in the vertex shader.
			This is necessary to pass them to the fragment shader for interpolation across the surface of the square.
		*/
		static std::array<VkVertexInputAttributeDescription, 3> RetrieveAttributeDescriptions()
		{
			/*
				An attribute description struct describes how to extract a vertex attribute from a chunk of vertex data originating from a binding description. We have three
				attributes, a position, a color and a tex coord. Thus, we need 3 attribute description structs.
			*/
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;
			attributeDescriptions[0].binding = 0; //Tells Vulkan from which binding the per-vertex data comes from.
			attributeDescriptions[0].location = 0; //References the location directive of the input in the vertex shader. In this case, the input in the vertex shader with location 0 is the position, which has two 32-bit float components.
			/*
				Format describes the type of data of the attribute. A bit confusingly, the formats are specified using the same enumeration as the color formats. The following shader
				types and formats are commonly used together:

				- float: VK_FORMAT_R32_SFLOAT
				- vec2: VK_FORMAT_R32G32_SFLOAT
				- vec3: VK_FORMAT_R32G32B32_SFLOAT
				- vec4: VK_FORMAT_R32G32B32A32_SFLOAT

				As you can see, you should use the format where the amount of color channels matches the number of components in the shader data type. It is allowed to use more
				channels than the number of components in the shader, but they will be silently discarded. If the number of channels is lower than the number of components, then
				the BGA components will use the default values of (0, 0, 1). The color type (SFLOAT, UINT, SINT) and bit width should also match the type of the shader input. See
				the following examples:

				- ivec2: VK_FORMAT_R32G32_SINT, a 2 component vector of 32-bit signed integers.
				- uvec4: VK_FORMAT_R32G32B32A32_UINT, a 4 component vector of 32-bit unsigned integers.
				- double: VK_FORMAT_R64_SFLOAT, a double precision (64-bit) float.

				The format parameter implicitly defines the byte size of attribute data and the offset parameter specifies the number of bytes since the start of the per-vertex
				data to read from. The binding is loading one Vertex (our struct) at a time and the position attribute (m_Position) is at an offset of 0 bytes from the beginning
				of the struct. This is automatically calculated using the offsetof macro.
			*/
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, m_Position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, m_Color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, m_TexCoord);

			return attributeDescriptions;
		}
	};

	struct UniformBufferObject
	{
		glm::mat4 m_ModelMatrix; //The data in the GLM matrices is binary compatible with the way the shader expects it, so we can just memcpy a UniformBufferObject to a VkBuffer.
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;
	};

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

	//The support of a format depends on the tiling mode and usage, so we must include these as parameters. The support of a format can be queried using the vkGetPhysicalDeviceFormatProperties function.
	static VkFormat FindSupportedFormat(const VkPhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			/*
				The VkFormatProperties struct contains 3 fields:

				- linearTilingFeatures: Use cases that are supported with linear tiling.
				- optimalTilingFeatures: Use cases that are supported with optimal tiling.
				- bufferFeatures: Use cases that are supported for buffers.

				Only the first two are relevant here, and the one we check depends on the tiling parameter of the function.
			*/
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("Failed to find supported format.\n"); //If none of the candidate formats support the desired usage, then we can either return a special value or simply throw an exception.
	}

	static VkFormat FindDepthFormat(const VkPhysicalDevice& physicalDevice)
	{
		/*
			We will now use the FindSupportedFormat function to help select a format with a depth component that supports usage as depth attachment. Make sure to use the
			VK_FORMAT_FEATURE_ flag instead of VK_IMAGE_USAGE_ in this case. All of these candidate formats contain a depth component, but the latter two also contain
			a stencil component. We won't be using that yet, but we do need to take that into account when performing layout transitions on image with these formats.
		*/
		return FindSupportedFormat(physicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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
			std::cout << "Successfully allocated Buffer Memory.\n";
		}

		/*
			Once successful, we can associate this memory with the buffer directly. The first three parameters are straightforward, and the fourth parameter is the offset
			within the region of memory. Since this memory is allocated specifically for this: the buffer we are creating, the offset is simply 0. If the offset is non-zero,
			then it is required to be divisible by memoryRequirements.alignment.
		*/
		vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
	}

	//Starts a new command buffer.
	static VkCommandBuffer BeginSingleTimeCommands(const VkDevice& logicalDevice, const VkCommandPool& commandPool)
	{
		/*
			Memory transfer operations are executed using command buffers, just like drawing commands. Therefore, we must allocate a temporary command buffer. You may wish
			to create a seperate command pool for these kinds of short-lived buffers, because the implementation may then be able to apply memory allocation optimizations.
			You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case. This indicates that the command buffers allocated from said pool
			will be short lived (reset/freed in a short timeframe).
		*/
		VkCommandBufferAllocateInfo commandBufferInfo{};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //Primary command buffers can be submitted to a queue for execution, but cannot be called from other command buffers.
		commandBufferInfo.commandPool = commandPool;
		commandBufferInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(logicalDevice, &commandBufferInfo, &commandBuffer);

		/*
			We're only going to use the command buffer once and wait with returning from the function until the copy operation has finished executing. Thus, its good practice
			to tell the driver about our intent using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
		*/
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo); //Begin recording of a command buffer.

		return commandBuffer;
	}

	//Ends and submits a command buffer for execution aftrer recording.
	static void EndSingleTimeCommands(VkCommandBuffer commandBuffer, const VkCommandPool& commandPool, const VkDevice& logicalDevice, const VkQueue& queue)
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkEndCommandBuffer(commandBuffer);

		//Execute the command buffer to complete the operation.
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		/*
			Unlike drawing commands, there are no events we need to wait on in transfer operations. We just want to execute the transfers on the buffers immediately. There are
			again two possible ways to wait on this transfer to complete. We could use a fence and wait with vkWaitForFences, or simply wait for the transfer queue to become idle
			with vkQueueWaitIdle. A fence would allow you to schedule multiple transfers simultaneously and wait for all of them to complete, instead of executing one at a time.
			That may give the driver more opportunities to optimize. 
		*/
		vkQueueWaitIdle(queue);

		//Don't forget to clean up the command buffer used for the transfer operation.
		vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
	}

	static bool HasStencilComponent(VkFormat format)
	{
		//Check if the chosen depth format contains a stencil format.
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	static void CopyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size, const VkCommandPool& commandPool, const VkDevice& logicalDevice, const VkQueue& queue)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(logicalDevice, commandPool);

		/*
			Contents of buffers are transferred using the vkCmdCopyBuffer command. It takes the source and destination buffers as arguments, and an array of region to copy.
			The regions are defined in VkBufferCopy structs and consists of a source buffer offset, destination buffer offset and size. It is not possible to specify VK_WHIOLE_SIZE here
			unlike the vkMapMemory command.
		*/
		VkBufferCopy copyRegionInfo{};
		copyRegionInfo.srcOffset = 0; //Optional
		copyRegionInfo.dstOffset = 0; //Optional
		copyRegionInfo.size = size;
		vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegionInfo);
		//As the command buffer only contains the copy command, we can stop recording after that.

		EndSingleTimeCommands(commandBuffer, commandPool, logicalDevice, queue);
	}
}

template<>
struct std::hash<Crescent::Vertex>
{
	size_t operator()(Crescent::Vertex const& vertex) const
	{
		return ((std::hash<glm::vec3>()(vertex.m_Position) ^
			(std::hash<glm::vec3>()(vertex.m_Color) << 1)) >> 1) ^
			(std::hash<glm::vec2>()(vertex.m_TexCoord) << 1);
	}
};