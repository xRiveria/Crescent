#include "VulkanTexture.h"
#include <stdexcept>
#include <iostream>

namespace Crescent
{
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