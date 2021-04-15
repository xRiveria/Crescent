#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanPipeline
	{
	public:
		VulkanPipeline(const VkFormat& swapchainImageFormat, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkExtent2D* swapchainExtent, VkDescriptorSetLayout* descriptorSetLayout);
		void DestroyPipelineInstance();

		void CreateRenderPass();
		void CreateGraphicsPipeline();

		VkRenderPass* RetrieveRenderPass() { return &m_RenderPass; }
		VkPipeline* RetrievePipeline() { return &m_Pipeline; }
		VkPipelineLayout* RetrievePipelineLayout() { return &m_PipelineLayout; }

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