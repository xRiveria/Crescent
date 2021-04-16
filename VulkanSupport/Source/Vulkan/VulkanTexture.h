#pragma once
#include <vulkan/vulkan.h>
#include <string>

namespace Crescent
{
	class VulkanTexture
	{
	public:
		//For textures uploaded by the user.
		VulkanTexture(const std::string& filePath, VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkSampleCountFlagBits sampleCount, VkFormat imageFormat, const VkImageAspectFlags& imageAspectFlags, VkCommandPool* commandPool, VkQueue* queue);
		//For textures whose data are populated by the implementation, such as those used by the swapchain.
		VulkanTexture(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, const VkImage& image, const VkFormat& imageFormat, const VkImageAspectFlags& imageAspectFlags);
		//For textures to be created on the spot.
		VulkanTexture(const int& textureWidth, const int& textureHeight, VkSampleCountFlagBits sampleCount, VkFormat format, const VkImageAspectFlags& imageAspectFlags, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice);
		
		void DeleteTextureViewInstance();
		void DeleteTextureImageInstance();
		void DeleteTextureMemoryInstance();
		void DeleteTextureSamplerInstance();
		void DestroyAllTextureInstances();

		void CreateTexture(const int& textureWidth, const int& textureHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void CreateTextureView();
		void CreateTextureSampler();

		void CopyBufferToImage(VkBuffer buffer, uint32_t imageWidth, uint32_t imageHeight);
		void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, const VkCommandPool& commandPool, const VkQueue& queue);

		VkImage* RetrieveTexture() { return &m_Texture; }
		VkImageView* RetrieveTextureView() { return &m_TextureView; }
		VkDeviceMemory* RetrieveTextureMemory() { return &m_TextureMemory; }
		VkSampler* RetrieveTextureSampler() { return &m_TextureSampler; }

	private:
		void GenerateMipmaps(VkImage texture, VkFormat imageFormat, int32_t textureWidth, int32_t textureHeight, uint32_t mipLevels);

	private:
		VkImage m_Texture;
		VkDeviceMemory m_TextureMemory;
		VkImageView m_TextureView;
		VkSampler m_TextureSampler;
		uint32_t m_MipmapLevels = 1;
		VkSampleCountFlagBits m_SampleCount = VK_SAMPLE_COUNT_1_BIT;

		VkFormat m_TextureFormat;
		VkImageAspectFlags m_TextureTypeFlag; //Identifies the resource usage, such as Color, Depth or Stencil Texture etc.

		VkQueue* m_Queue;
		VkCommandPool* m_CommandPool;
		VkDevice* m_LogicalDevice;
		VkPhysicalDevice* m_PhysicalDevice;
	};
}