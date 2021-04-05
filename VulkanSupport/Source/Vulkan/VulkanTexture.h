#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanTexture
	{
	public:
		VulkanTexture(VkDevice* logicalDevice, const VkImage& image, const VkFormat& imageFormat, const VkImageAspectFlags& imageAspectFlags);

		VkImage* RetrieveTexture() { return &m_Texture; }
		VkImageView* RetrieveTextureView() { return &m_TextureView; }

	private:
		void CreateTextureView();

	private:
		VkImage m_Texture;
		VkImageView m_TextureView;
		VkFormat m_TextureFormat;
		VkImageAspectFlags m_TextureTypeFlag; //Identifies the resource usage, such as Color, Depth or Stencil Texture etc.

		VkDevice* m_LogicalDevice;
	};
}