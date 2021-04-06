#pragma once
#include <vulkan/vulkan.h>
#include <string>

namespace Crescent
{
	class VulkanTexture
	{
	public:
		//For textures uploaded by the user.
		VulkanTexture(const std::string& filePath, VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkFormat imageFormat, const VkImageAspectFlags& imageAspectFlags);

		//For textures whose data are populated by the implementation, such as those used by the swapchain.
		VulkanTexture(VkDevice* logicalDevice, const VkImage& image, const VkFormat& imageFormat, const VkImageAspectFlags& imageAspectFlags);

		VkImage* RetrieveTexture() { return &m_Texture; }
		VkImageView* RetrieveTextureView() { return &m_TextureView; }

	private:
		void CreateTexture(const int& textureWidth, const int& textureHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void CreateTextureView();

	private:
		VkImage m_Texture;
		VkDeviceMemory m_TextureMemory;
		VkImageView m_TextureView;
		VkFormat m_TextureFormat;
		VkImageAspectFlags m_TextureTypeFlag; //Identifies the resource usage, such as Color, Depth or Stencil Texture etc.

		VkDevice* m_LogicalDevice;
		VkPhysicalDevice* m_PhysicalDevice;
	};
}