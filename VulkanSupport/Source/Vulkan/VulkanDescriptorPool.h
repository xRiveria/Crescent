#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include <memory>
#include "VulkanTexture.h"

namespace Crescent
{
	class VulkanDescriptorPool
	{
	public:
		VulkanDescriptorPool(std::vector<std::shared_ptr<VulkanTexture>>* swapchainImages, VkDevice* logicalDevice);

	private:
		void CreateDescriptorPool();

	private:
		VkDescriptorPool m_DescriptorPool;

		VkDevice* m_LogicalDevice;
		std::vector<std::shared_ptr<VulkanTexture>>* m_SwapchainImages;
	};
}