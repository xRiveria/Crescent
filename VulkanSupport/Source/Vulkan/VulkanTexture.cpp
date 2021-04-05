#include "VulkanTexture.h"
#include <stdexcept>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "../../Vendor/stb_image/stb_image.h"

namespace Crescent
{
	VulkanTexture::VulkanTexture(const std::string& filePath, VkDevice* logicalDevice,            VkFormat imageFormat, VkImageUsageFlags imageUsage, VkMemoryPropertyFlags imageProperties,
		VkImage& image, VkDeviceMemory& imageMemory, const VkImageAspectFlags& imageAspectFlags) : m_LogicalDevice(logicalDevice), m_Texture(image), m_TextureFormat(imageFormat), m_TextureTypeFlag(imageAspectFlags)
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

		//Create Buffer.

		//We can then directly copy the pixel values that we got from the image loading library to the buffer.
		void* dataBuffer;
		vkMapMemory(*m_LogicalDevice, stagingBufferMemory, 0, imageSize, 0, &dataBuffer); //After a successful call to vkMapMemory, the memory object is considered to be currently host mapped.
		memcpy(dataBuffer, pixels, static_cast<uint32_t>(imageSize));
		vkUnmapMemory(*m_LogicalDevice, stagingBufferMemory);

		//Clean up the original pixel array once mapped.
		stbi_image_free(pixels);

		//Create Image.

		CreateTextureView();
	}

	VulkanTexture::VulkanTexture(VkDevice* logicalDevice, const VkImage& image, const VkFormat& imageFormat, const VkImageAspectFlags& imageAspectFlags)
		: m_LogicalDevice(logicalDevice), m_Texture(image), m_TextureFormat(imageFormat), m_TextureTypeFlag(imageAspectFlags)
	{
		CreateTextureView();
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