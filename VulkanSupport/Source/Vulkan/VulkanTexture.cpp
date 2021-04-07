#include "VulkanTexture.h"
#include <stdexcept>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "../../Vendor/stb_image/stb_image.h"
#include "VulkanUtilities.h"

namespace Crescent
{
	VulkanTexture::VulkanTexture(const std::string& filePath, VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkFormat imageFormat, const VkImageAspectFlags& imageAspectFlags) : m_LogicalDevice(logicalDevice), m_PhysicalDevice(physicalDevice), m_TextureFormat(imageFormat), m_TextureTypeFlag(imageAspectFlags)
	{
		/*
			The stbi_load function takes the file path and number of channels to load as arguments. The STBI_rgb_alpha value forces the the image to be loaded with an alpha channel,
			even if it doesn't have one, which is nice for consistency with other textures in the future. The middle three parameters are output for width, height and actual
			number of channels in the image. The pointer that is returned is the first element in an array of pixel values. The pixels are load out row by row with 4 bytes
			per pixel in the case of STBI_rgb_alpha for a total of textureWidth * textureHeight * 4 values.
		*/
		int textureWidth, textureHeight, textureChannels;
		stbi_uc* pixels = stbi_load(filePath.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = textureWidth * textureHeight * 4; //VkDeviceSize represents device memory size and offset values. It is internally a uint64_t.

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image with path: " + filePath);
		}

		//We're going to create a buffer in host visible memory so that we can use vkMapMemory and copy the pixels loaded from our image library to it. 
		//This buffer should not only be in host visible memory but also be able to be used as a transfer source so we can copy it to an image eventually.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, *m_LogicalDevice, *m_PhysicalDevice);

		//We can then directly copy the pixel values that we got from the image loading library to the buffer.
		void* dataBuffer;
		vkMapMemory(*m_LogicalDevice, stagingBufferMemory, 0, imageSize, 0, &dataBuffer); //After a successful call to vkMapMemory, the memory object is considered to be currently host mapped.
		memcpy(dataBuffer, pixels, static_cast<uint32_t>(imageSize));
		vkUnmapMemory(*m_LogicalDevice, stagingBufferMemory);

		//Clean up the original pixel array once mapped.
		stbi_image_free(pixels);

		//Create our Vulkan image object. This will be a 4 component, 32-bit unsigned normalized format that has 8 bits per component.
		CreateTexture(textureWidth, textureHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Texture, m_TextureMemory);
		CreateTextureView();

		/*
			Now, we are going to copy the staging buffer in host visible memory to the texture image. This involves 2 steps:
				1) Transitioning the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
				2) Executing the buffer-to-image operation.
			The image was created with the VK_IMAGE_UNDEFINED layout, so that layout should be specified as the old layout when transitioning the texture image. Remember that
			we can do this because we don't care about its contents prior to the copy operation.
		*/
	}

	VulkanTexture::VulkanTexture(VkDevice* logicalDevice, const VkImage& image, const VkFormat& imageFormat, const VkImageAspectFlags& imageAspectFlags)
		: m_LogicalDevice(logicalDevice), m_Texture(image), m_TextureFormat(imageFormat), m_TextureTypeFlag(imageAspectFlags)
	{
		CreateTextureView();
	}

	void VulkanTexture::DeleteTextureInstance()
	{
		vkDestroyImageView(*m_LogicalDevice, m_TextureView, nullptr);
		//vkDestroyImage(*m_LogicalDevice, m_Texture, nullptr);
		//vkFreeMemory(*m_LogicalDevice, m_TextureMemory, nullptr);
	}

	//Represents an abstract texture creation function. The width, height, format, tiling mode, usage and memory properties parameters are different as they vary between images.
	void VulkanTexture::CreateTexture(const int& textureWidth, const int& textureHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		/*
			Although we could set up the shader to access the pixel values directly in the buffer, its better to use image objects in Vulkan for this purpose. For one, image objects
			will make it easier and faster to retrieve colors by allowing us to use 2D coordinates. Pixels within an image object are known as texels.

			The image type, specified in the imageType field, tells Vulkan with what kind of coordinate system the texels in the image are going to be addressed. It is
			possible to create 1D, 2D or 3D images. One dimensional images can be used to store an array or gradient, two dimensional images are mainly used for textures,
			and 3 dimensional images can be used to store voxel volumes for example. The extent field specifies the dimensions of the image, basically how many texels there are
			on each axis. That's why the depth must be 1 instead of 0. Our texture will not be an array and we won't be using mipmapping for now.
		*/
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(textureWidth);
		imageInfo.extent.height = static_cast<uint32_t>(textureHeight);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1; //A progressively lower resolution representation of the previous one for optimization.
		imageInfo.arrayLayers = 1;
		//Vulkan supports many possible image formats, but we should use the same format for the texels as the pixels in the buffer. Otherwise, the copy operation will fail.
		imageInfo.format = format;

		/*
			The tiling field can have one of two values:
			
			- VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major like our pixels array.
			- VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined order for optimal access.

			Unlike the layout of an image, the tiling mode cannot be changed at a later time. If you want to be able to directly access texels in the memory of an image,
			then you must use the VK_IMAGE_TILING_LINEAR. We will be using a staging buffer instead of a staging image, so this won't be necessary. We will be using
			VK_IMAGE_TILING_OPTIMAL for efficient access from the shader.
		*/
		imageInfo.tiling = tiling;
		/*
			There are only two possible values for the initialLayout of an image:

			- VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
			- VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.

			There are few situations where it is necessary for the texels to be preserved during the first transition. One example, however, would be if you wanted to use an
			image as a staging image in combinationm with the VK_IMAGE_TILING_LINEAR layout. In that case, you would want to upload the texel data to it and then
			transition the image to be a transfer source without losing the data. In our case, however, we're going to first transition the image to be a transfer
			destination and copy the texel data to it from a buffer object, so we don't need this property anymore and can safely use VK_IMAGE_LAYOUT_UNDEFINED.
		*/
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		/*
			The usage field has the same semantics as the one during buffer creation. The image is going to be used as a destination for the buffer copy, so it should be
			setup as a transfer destination. We also want to be able to access the image from the shader to color our mesh, so the usage should include VK_IMAGE_USAGE_SAMPLED_BIT.
		*/
		imageInfo.usage = usage;
		//The image will only be used by one queue family: the one that supports graphics (and therefore also) transfer operations.
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		/*
			The samples flag is related to multisampling. This is only relevant for images that will be used as attachments, so stick to one sample. There are some
			optional flags for images that are related to sparse images. Sparse images are images where only certain regions are backed by memory. If you were
			using a 3D texture for a voxel terrain, for example, we could use this to avoid allocating memory to store large amounts of "air" values. We won't be using it,
			so lets leave it at its default value of 0.
		*/
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0; //Optional
		/*
			The image is created using vkCreateImage, which doesn't have any particularly noteworthy parameters. It is possible that the VK_FORMAT_R8G8B8A8_SRGB format'
			is not supported by the graphics hardware. Thus, we should have a list of acceptable alternatives and go with the best one that is supported. However,
			support for this particular format is so widespread that we will skip this step. Using different formats would also require annoying conversions. We will get
			back to this when doing up depth-buffers.
		*/
		if (vkCreateImage(*m_LogicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image.");
		}
		/*
			Allocating memory for an image works exactly in the same way as allocating memory for a buffer. Use vkGetImageMemoryRequirements instead of vkGetBufferMemoryRequirements,
			and use vkBindImageMemory instead of vkBindBufferMemory.
		*/
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(*m_LogicalDevice, image, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties, *m_PhysicalDevice);

		if (vkAllocateMemory(*m_LogicalDevice, &allocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory.");
		}
		
		vkBindImageMemory(*m_LogicalDevice, image, imageMemory, 0);
	}

	void VulkanTexture::CreateTextureView()
	{
		/*
			An image view is literally a view into an image and is required to use an VkImage. It describes how to access the image and which part of the image to access.
			For example, should it be treated as a 2D depth texture without any mipmapping levels?
		*/
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = m_Texture;
		//The viewType and format fields specifies how the image data should be interpreted. The viewType parameter allows you to treat images as 1D, 2D or 3D textures and cubemaps.
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = m_TextureFormat;
		//The components fields allow you to swizzle the color channels around. 
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		/*
			The subresourceRange fields describe what the image's purpose is and which part of the image should be accessed. Our images will be used as color targets
			without any mipmapping levels or multiple layers, for example.
		*/
		imageViewInfo.subresourceRange.aspectMask = m_TextureTypeFlag;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		//If this was a steoreographic 3D application, we would create a swapchain with multiple layers. We can then create multiple image views for each image representing
		//the views for the left and right eyes by accessing different layers.
		imageViewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(*m_LogicalDevice, &imageViewInfo, nullptr, &m_TextureView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Image View Texture.");
		}
		else
		{
			std::cout << "Successfully created Image View.\n";
		}
	}
}