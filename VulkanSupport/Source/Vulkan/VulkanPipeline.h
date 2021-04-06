#pragma once
#include <vulkan/vulkan.h>

namespace Crescent
{
	class VulkanPipeline
	{
	public:
		VulkanPipeline(VkFormat& swapchainImageFormat);

	private:
		void CreateRenderPass();
		void CreateGraphicsPipeline();

	private:
		VkRenderPass m_RenderPass;
		VkPipeline m_GraphicsPipeline;

		VkFormat m_SwapchainImageFormat;
	};
}