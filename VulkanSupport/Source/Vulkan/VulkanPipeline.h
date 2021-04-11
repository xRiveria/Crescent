#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanPipeline
	{
	public:
		VulkanPipeline(const VkFormat& swapchainImageFormat, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkExtent2D* swapchainExtent, VkDescriptorSetLayout* descriptorSetLayout);
		void DestroyPipelineInstance();

		VkRenderPass* RetrieveRenderPass() { return &m_RenderPass; }

	private:
		void CreateRenderPass();
		void CreateGraphicsPipeline();

	private:
		VkRenderPass m_RenderPass;
		VkPipeline m_Pipeline;
		VkPipelineLayout m_PipelineLayout;

		VkDescriptorSetLayout* m_DescriptorSetLayout;
		VkExtent2D* m_SwapchainExtent;
		VkFormat m_SwapchainImageFormat;
		VkPhysicalDevice* m_PhysicalDevice;
		VkDevice* m_LogicalDevice;
	};
}