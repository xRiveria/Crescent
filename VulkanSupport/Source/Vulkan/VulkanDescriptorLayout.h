#pragma once
#include "vulkan/vulkan.h"

namespace Crescent
{
	class VulkanDescriptorLayout
	{
	public:
		VulkanDescriptorLayout(VkDevice* logicalDevice);
		void DestroyDescriptorLayoutInstance();

		VkDescriptorSetLayout* RetrieveDescriptorSetLayout() { return &m_DescriptorSetLayout; }

	private:
		void CreateDescriptorSetLayout();

	private:
		VkDescriptorSetLayout m_DescriptorSetLayout;

		VkDevice* m_LogicalDevice;
	};
}