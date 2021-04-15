#include "VulkanSwapchain.h"
#include "VulkanUtilities.h"
#include <stdexcept>
#include <iostream>

namespace Crescent
{
	VulkanSwapchain::VulkanSwapchain(VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkSurfaceKHR* presentationSurface, GLFWwindow* window) 
		: m_PhysicalDevice(physicalDevice), m_LogicalDevice(logicalDevice), m_Surface(presentationSurface), m_Window(window)
	{
		CreateSwapchain();
	}

	void VulkanSwapchain::DestroySwapchainInstance()
	{
		//
	}

	/*
		We have previously queried for swapchain support. If the conditions were met, then the support is definitely sufficient. However, there may still be many more modes
		with varying optimality. We will now try to determine for the best possible swapchain based on 3 types of settings:

		- Surface Mode (Color Depth)
		- Presentation Mode (Conditions for "swapping" images to the screen).
		- Swap Extent (Resolution of images in the swapchain).
	*/

	VkSurfaceFormatKHR VulkanSwapchain::SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaliableFormats)
	{
		/*
			Each VkSurfaceFormatKHR entry contains a format and a colorSpace member. The format member specifies color channels and types. For example, VK_FORMAT_B8G8R8A8_SRGB
			means that we store the B, R, G and Alpha channels in that order with an 8 bit unsigned integer for a total of 32 bits per pixel. 
			
			The colorSpace member indicates if the SRGB color space is supported or not using the VK_COLOR_SPACE_SRGB_NONLINEAR_KHR flag. For the color space, we will use 
			SRGB if it is avaliable because it results in more accurately perceivec colors. It is pretty much the standard color space for images, like the textures we 
			will use later on. Because of that, we should also use an SRGB color format, a common one being VK_FORMAT_B8G8R8A8_SRGB.
		*/

		for (const VkSurfaceFormatKHR& avaliableFormat : avaliableFormats)
		{
			if (avaliableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && avaliableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return avaliableFormat;
			}
		}

		//If the above fails, we could start ranking the avaliable formats based on how good they are. In most cases, it is okay to settle with the first format specified.

		return avaliableFormats[0];
	}

	VkPresentModeKHR VulkanSwapchain::SelectSwapchainPresentationMode(const std::vector<VkPresentModeKHR>& avaliablePresentationModes)
	{
		/*
			The presentation mode is arguably the most important setting for the swapchain, because it represents the actual conditions for showing images to the screen.
			There are 4 possible modes avalible in Vulkan:

			- VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away, which may result in tearing.

			- VK_PRESENT_MODE_FIFO_KHR: The swapchain is a queue where the display takes an image from the front of the queue when the display is refreshed and the
			program inserts rendered images at the back of the queue. If the queue is full, then the program has to wait. This is most similarly to vertical sync (VSync)
			found in modern games. The moment the display is refrshed is known as "vertical blank".

			- VK_PRESENT_MODE_FIFO_RELAXED_KHR: The mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank.
			Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives. This may result in visible tearing.

			- VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of blocking the application when the queue is full, the images that are
			already queued are simply replaced with the newer ones. This mode can be used to implement triple buffering, which allows you to avoid tearing with its significantly
			less latency issues than standard vertical sync that uses double buffering. 

			VSync is a graphics technology that synchronizes the frame rate of a game and the refresh rate of the monitor. This tech was a way to deal with screen tearing,
			which is when your screen displays portions of multiple frames at one go, resulting in a display that appears split along a line, usually horizontally. Tearing
			occurs when the fresh rate of the monitor (how many times it updates per second) is not in sync with the amount of frames per second.

			Only the VK_PRESENT_MODE_FIFO_KHR mode is guarenteed to be avaliable, so we will again write a function that looks for the best one avaliable.
		*/

		for (const VkPresentModeKHR& presentationMode : avaliablePresentationModes)
		{
			//Triple buffering is a very nice tradeoff as it allows us to prevent tearing whilst still maintaining a fairly low latency by rendering new images that are
			//as up-to-date as possible right until the vertical blank.
			if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return presentationMode;
			}
		}

		//If our preferred optrion isn't avaliable, just return the default one.
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapchain::SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		/*
			The swap extent is the resolution of the swapchain images and its almost always exactly equal to the resolution of the window that we're drawing to in pixels.
			The range of the possible resolutions is defined in the VkSurfaceCapabilitiesKHR structure. Vulkan tells us to match the resolution of the window by setting
			the width and height in the currentExtent member. However, some window managers do allow us to differ here and this is indicated by setting the width
			and height in currentExtent to a special value: the maximum value of uint32_t. In that case, we will pick the resolution that best matches the window without the
			minImageExtent and maxImageExtent bounds. We must specify the resolution in the correct unit.

			GLFW uses 2 units when measuring sizes: pixels and screen coordinates. For example, the resolution (width/height) that we specified earlier when creating the
			window were measured in screen coordinates. However, Vulkan works with pixels, so the swapchain extent must be specified in pixels. Unfortunately,
			if you are using a high DPI display (like Apple's retina display), screen coordinates don't correspond to pixels. Instead, due to the higher pixel density,
			the resolution of the window in pixel will be larger than the resolution in screen coordinates. Thus, if Vulkan doesn't fix the swap extent for us, we can't
			just use the original width and height. Instead, we must use glfwGetFramebufferSize to query the resolution of the window in pixel before matching it against
			the minimum and maximum image extent.
		*/

		if (capabilities.currentExtent.width != UINT32_MAX) //The maximum number that we can store with an unsigned 32-bit integer.
		{
			return capabilities.currentExtent;
		}
		else //We pick the resolution that best matches the window.
		{
			int windowWidth, windowHeight;
			glfwGetFramebufferSize(m_Window, &windowWidth, &windowHeight);
			VkExtent2D actualExtent = { static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight) };

			//The min and max functions are used to clamp the values of width and height between the allowed minimum and maximum extents that are supported by the implementation.
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void VulkanSwapchain::CreateSwapchain()
	{
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(*m_PhysicalDevice, *m_Surface);

		VkSurfaceFormatKHR surfaceFormat = SelectSwapchainSurfaceFormat(swapchainSupport.m_Formats);
		VkPresentModeKHR presentationMode = SelectSwapchainPresentationMode(swapchainSupport.m_PresentationModes);
		VkExtent2D swapExtent = SelectSwapExtent(swapchainSupport.m_Capabilities);

		/*
			Aside from these properties, we also have to decide how many images we would like to have in the swapchain. The implementation specifies the minimum number
			that is requires to function. However, simply sticking to this minimum means that we may sometimes have to wait on the driver to complete internal operations
			before we can acquire another image to render to. Therefore, it is recommended to request at least 1 more image than the minimum.
		*/
		uint32_t imageCount = swapchainSupport.m_Capabilities.minImageCount + 1;
		//We should also make sure to not exceed the maximum number of images while doing this, while 0 is a special value that means that there is no maximum.
		if (swapchainSupport.m_Capabilities.maxImageCount > 0 && imageCount > swapchainSupport.m_Capabilities.maxImageCount) //If image support count is more than 0 and our requested count is more than the supported count...
		{
			imageCount = swapchainSupport.m_Capabilities.maxImageCount; //Set to max.
		}

		VkSwapchainCreateInfoKHR swapchainCreationInfo{};
		swapchainCreationInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreationInfo.surface = *m_Surface;
		swapchainCreationInfo.minImageCount = imageCount;
		swapchainCreationInfo.imageFormat = surfaceFormat.format;
		swapchainCreationInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapchainCreationInfo.imageExtent = swapExtent;

		/*
			imageArrayLayers specifies the amount of layers each image consists of. This is always 1 unless you are developing a stereoscopic 3D application. The
			imageUsage bit field specifies what kinds of operations we will be using the images in the swapchain for. Here, we're going to render directly to them,
			which means that they're used as color attachments. It is also possible that you will render images to a seperate image first to perform operations like
			post-processing. In that case, you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a swapchain image.
		*/

		swapchainCreationInfo.imageArrayLayers = 1;
		swapchainCreationInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		/*
			We now specify how to handle swapchain images that will be used across multiple queue families. That will be the case in our application if the graphics
			queue family is different from the presentation queue. We will be drawing on the images in the swapchain from the graphics queue and then submitting them
			on the presentation queue. There are 2 ways to handle images that are accessed from multiple queues:

			- VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explictly transferred before using it in another queue family.
			This offers the best performance.

			- VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership transfers.

			Concurrent mode requires you to specify in advance between which queue families ownership will be shared using queueFamilyIndexCount and pQueueFamilyIndices parameters.
			If the graphics queue family and presentation queue family are the same, which will be the case on most hardware, we should stick to exclusive mode, because
			concurrent mode requires you to specify at least 2 distinct queue families.
		*/
		QueueFamilyIndices queueFamilies = QueryQueueFamilySupport(*m_PhysicalDevice, *m_Surface);
		uint32_t queueFamilyIndices[] = { queueFamilies.m_GraphicsFamily.value(), queueFamilies.m_PresentationFamily.value() };

		if (queueFamilies.m_GraphicsFamily != queueFamilies.m_PresentationFamily) //If the queue families are different, we use Concurrent mode.
		{
			swapchainCreationInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreationInfo.queueFamilyIndexCount = 2;
			swapchainCreationInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			swapchainCreationInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreationInfo.queueFamilyIndexCount = 0; //Optional. Not needed in Exclusive.
			swapchainCreationInfo.pQueueFamilyIndices = nullptr;
		}

		/*
			We can also specify that a certain transform be applied to images in the swapchain if it is supported (supportTransforms in capabilities), like a 90 degree
			closewise rotation or horizontal flip. To specify that you do not need any transformation, simply specify the current transformation.
		*/
		swapchainCreationInfo.preTransform = swapchainSupport.m_Capabilities.currentTransform;

		//The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system. You will almost always want to simply ignore the alpha channel.
		//Hence, use VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
		swapchainCreationInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		/*
			The presentMode member speaks for itself. If the clipped member is set to VK_TRUE, then that means that we don't care about the color of pixels that are obscured,
			for example because another window is in front of them. Unless you really need to be able to read these pixels back and get predictable results, you will get 
			the best peformance by enabling clipping.
		*/
		swapchainCreationInfo.presentMode = presentationMode;
		swapchainCreationInfo.clipped = VK_TRUE;

		/*
			That leaves one last field, oldSwapChain. With Vulkan, it is possible that your swapchain becomes invalid or unoptimized while your application is running, for example
			because the window was resized. In that case, the swapchain actually needs to be recreated from scratch and a reference to the old one must be specified in this field.
			This is rather complex, so for now we will assume that we will only ever create one swapchain.
		*/
		swapchainCreationInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(*m_LogicalDevice, &swapchainCreationInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Swapchain.\n");
		}
		else
		{
			std::cout << "Successfully created Swapchain.\n";
		}

		/*
			Remember that we only specified a minimum number of images in the swapchain, so the implementation is allowed to create a swapchain with more.
			Thus, we will first query the final number of images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to retrieve the handles.
		*/
		vkGetSwapchainImagesKHR(*m_LogicalDevice, m_Swapchain, &imageCount, nullptr);

		//Temporary buffer to store the image handles.
		std::vector<VkImage> swapchainImages;
		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(*m_LogicalDevice, m_Swapchain, &imageCount, swapchainImages.data());

		for (int i = 0; i < swapchainImages.size(); i++)
		{
			//Our swapchain textures will be used as color attachments.
			m_SwapchainTextures.push_back(std::make_shared<VulkanTexture>(m_LogicalDevice, m_PhysicalDevice, swapchainImages[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT));
		}

		//Store the format and swap extent for future uses.
		m_SwapchainFormat = surfaceFormat.format;
		m_SwapchainExtent = swapExtent;
	}

	void VulkanSwapchain::CreateMultisampledColorBufferResources()
	{
		/*
			We will now create a multisampled color buffer. Note that we will be using m_MSSASamples here as a function parameter to pass to the texture creation function. 
			We're also using only 1 mip level, since this is enforced by the Vulkan specification in case of images with more than one sample per pixel. Also, this 
			color buffer doesn't need mipmaps since its not going to be used as a texture.
		*/
		//VkFormat colorFormat = m_SwapchainFormat;
		//m_MultisampledColorBufferTexture = std::make_shared<VulkanTexture>(m_SwapchainExtent.width, m_SwapchainExtent.height, 1, )
	}

	void VulkanSwapchain::CreateDepthBufferResources(VkCommandPool* commandPool, VkQueue* queue)
	{
		/*
			The issue now is that the fragments of, for example, a lower square are drawn over the dragments of a upper square simply because it comes later in the index array. There
			are two ways to solve this: 1) Sort all of the draw calls by back to front or, 2) Use depth testing with a depth buffer.

			The first approach is commonly used for drawing transparent objects, because order-independant transparency is a very difficult challenge to solve. However, the problem
			of ordering fragments by depth is more commonly solved using a depth buffer. A depth buffer is an additional attachment that stores the depth for every position,
			just like the color attachment stores the color of every position. Everytime the rasterizer produces a fragment, the depth test will check if the new fragment is closer
			than the previous one.

			If it isn't, then the new fragment is discarded. A fragment that passes the depth test will write its own depth to the depth buffer. It is possible to manipulate this
			value directly from the fragment shader, just like how you can manipulate the color output.

			A depth attachment is based on an image just like the color attachment. The difference is that the swapchain will not automatically create depth images for us. We
			need only a single depth image, because only one draw operation is running at once. The depth image will thus once again require the trifecta of resources: image, memory and image view.

			Creating a depth image is fairly straightforward. It should have the same resolution as the color attachment, defined by the swapchain extent, an image usage appropriate
			for a depth attachment, optimal tiling and device local memory. The only question is: what is the right format for a depth image? The format must contain a depth component
			specified by _D??__ in the VK_FORMAT_.

			Unlike the texture image, we don't necessarily need a specific format, because we won't be directly accessing the texels from the program. It just needs to have a
			reasonable accuracy, at least 24 bits is common in real-world applications. THere are several formats that fit this requirement:

			- VK_FORMAT_D32_SFLOAT: 32-bit float for depth.
			- VK_FORMAT_D32_SFLOAT_S8_UINT: 32-bit signed float and 8-bit unsigned stencil component. 
			- VK_FORMAT_D24_UNORM_S8_UINT: 24-bit float for depth and 8-bit stencil component.

			We could simply go for VK_FORMAT_D32_SFLOAT, because support for it is extremely common, but its nice to add some extra flexibility to our application where possible.
			We're going to write a function named FindSupportedDepthFormat that takes a list of condidate formats in order from the most desirable to least desirable, and check
			which is the first one that is supported.
		*/

		VkFormat depthFormat = FindDepthFormat(*m_PhysicalDevice); //Find a depth format.
		m_DepthTexture = std::make_shared<VulkanTexture>(m_SwapchainExtent.width, m_SwapchainExtent.height, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_LogicalDevice, m_PhysicalDevice);
		
		/*
			And that is it for creating the depth image. We do not need to map it or copy another image to it, because we're going to clear it at the start of the render pass like the color attachment.
			We do also do not need to explicitly transition the layout of the image to a depth attachment because we will take care of this in the render pass. However, for completeness sake, 
			we will still do so. Here, the undefined layout can be used as initial layout, because there is no existing depth image contents that matter.
		*/
		m_DepthTexture->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, *commandPool, *queue);
	}

	void VulkanSwapchain::CreateFramebuffers(VkRenderPass* renderPass)
	{
		/*
			We have set up the render pass to expect a single framebuffer with the same format as the swapchain images, but we haven't created any yet. The attachments specified 
			during render pass creation are bound by wrapping them into a VkFramebuffer object. A framebuffer object references all of the VkImageView objects that represent 
			the attachments. In our case, there will only be a single one: the color attachment. However, the image that we have to use for the attachment depends on which 
			image the swapchain returns when we retrieve one for presentation. That means that we have to create a framebuffer for all of the images in the swapchain and use the 
			one that corresponds to the retrieved image at drawing time.
		*/
		m_SwapchainFramebuffers.resize(m_SwapchainTextures.size());
		/*
			We will then interate through the image views and create framebuffers from them. As seen, the creation of framebuffers is quite straightforward. We first need to specify 
			which renderpass the framebuffer needs to be compatible with. You only use a framebuffer with the render passes that it is compatible with, which roughly means that 
			they use the same number and type of attachments. The attachmentCount and pAttachments parameters specify the VkImageView objects that should be bound to the respective 
			attachment descriptions in the renderpass pAttachments array. The width and height parameters are self explainatory and layers refer to the number of layers in image arrays.
			Our swapchain images are single images, so the number of layers is 1.
		*/
		for (size_t i = 0; i < m_SwapchainTextures.size(); i++)
		{
			std::array<VkImageView, 2> attachments = { *m_SwapchainTextures[i]->RetrieveTextureView(), *m_DepthTexture->RetrieveTextureView() };
			
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = *renderPass; //Tells the framebuffer about its attachment usage.
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_SwapchainExtent.width;
			framebufferInfo.height = m_SwapchainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(*m_LogicalDevice, &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create Framebuffer.\n");
			}
			else
			{
				std::cout << "Successfully created Framebuffer.\n";
			}
		}
	}

	void VulkanSwapchain::CreateUniformBuffers()
	{
		/*
			We will create the buffers here that contain the UBO data for the shaders. As we will be copying the data to the uniform buffer every frame, it doesn't really 
			make sense to have a staging buffer. It would just add extra overhead in this case and likely degrade performance instead of improving it. We should have multiple 
			buffers, because multiple frames may be in flight at the same time and we don't want to have to update the buffer in preparation of the next frame while a previous 
			one is still reading from it. We could either have a uniform buffer per frame or per swapchain image. However, since we need to refer to the uniform buffer 
			from the command buffer that we have per swapchain image, it makes the most sense to have a uniform buffer per swapchain image.

			We will write a seperate function that updates the uniform buffer with a new transformation every frame, so there will be no vkMapMemory here.
		*/
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		m_UniformBuffers.resize(m_SwapchainTextures.size());
		m_UniformBuffersMemory.resize(m_SwapchainTextures.size());

		for (size_t i = 0; i < m_SwapchainTextures.size(); i++)
		{
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i], *m_LogicalDevice, *m_PhysicalDevice);
		}
	}

	void VulkanSwapchain::CreateDescriptorSets(std::shared_ptr<VulkanTexture> texture, VkDescriptorSetLayout* descriptorLayout, VkDescriptorPool* descriptorPool)
	{
		//We will create one descriptor set for each swapchain image, all with the same layout.
		std::vector<VkDescriptorSetLayout> layouts(m_SwapchainTextures.size(), *descriptorLayout);

		VkDescriptorSetAllocateInfo descriptorSetInfo{};
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.descriptorPool = *descriptorPool;
		descriptorSetInfo.descriptorSetCount = static_cast<uint32_t>(m_SwapchainTextures.size());
		descriptorSetInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(m_SwapchainTextures.size());
		//The vkAllocateDescriptorSets will allocate descriptor sets each with one uniform buffer descriptor.
		if (vkAllocateDescriptorSets(*m_LogicalDevice, &descriptorSetInfo, m_DescriptorSets.data()) != VK_SUCCESS) //Our array here needs to match the number of descriptor sets stated above.
		{
			throw std::runtime_error("Failed to allocate Descriptor Sets.\n");
		}
		else
		{
			std::cout << "Successfully allocated " << descriptorSetInfo.descriptorSetCount << " Descriptor Sets.\n";
		}

		//After allocation, we now need to configure each of the descriptors.
		for (size_t i = 0; i < m_SwapchainTextures.size(); i++)
		{
			//Descriptors that refer to buffers, like our uniform buffer descriptor, are configured with a VkDescriptorBufferInfo struct.
			//This structure specifies the buffer and the region within it that contains the data for the descriptor.
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject); //If we are overwriting the whole buffer, it is possible to use VK_WHOLE_SIZE for the range.
			
			//The final step is to bind the actual image and sampler resources to the descriptors in the descriptor set.
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = *texture->RetrieveTextureView();
			imageInfo.sampler = *texture->RetrieveTextureSampler();

			/*
				The configuration of the descriptors is updated using the vkUpdateDescriptorSets function, which takes an array of VkWriteDescriptorSet structs as parameter. 
				The first two fields specify the descriptor set to update and the binding. We gave our uniform buffer binding index 0. Remember that descriptors can be arrays,
				so we also need to specify the first index in the array that we want to update. We're not using an array, so the index is simply 0.

				We also need to specify the type of descriptor again. Its possible to update multiple descriptors at once in an array, starting at index dstArrayElement. The 
				descriptorCount field specifies how many array elements you wish to update.

				The last field references an array with descriptorCount structs that actually configure the descriptors. It depends on the type of descriptors which one of the 
				three you actually need to use. The pBufferInfo field is used for descriptors that refer to buffer data, pImageInfo is for descriptors that refer to image data and
				pTexelBufferView is used for descriptors that refer to buffer views. Our descriptor is based on buffers, so we're using pBufferInfo.
			*/

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_DescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; //Optional as we are using a buffer data here.
			descriptorWrites[0].pTexelBufferView = nullptr; //Optional as we are using a buffer data here.

			//The descriptors must be updated with the imageInfo descriptor info struct. Once done, they are finally ready to be used by the shaders/
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_DescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = nullptr;
			descriptorWrites[1].pImageInfo = &imageInfo;
			descriptorWrites[1].pTexelBufferView = nullptr;

			/*
				The updates are applied with vkUpdateDescriptorSets. It accepts two kinds of arrays as parameters: an array of VkWriteDescriptorSet and an array of VkCopyDescriptorSets. 
				The latter can be used to copy descriptors to each other as its name implies.
			*/
			vkUpdateDescriptorSets(*m_LogicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanSwapchain::RecreateSwapchainImageViews()
	{
		for (size_t i = 0; i < m_SwapchainTextures.size(); i++)
		{
			m_SwapchainTextures[i]->CreateTextureView();
		}
	}
}