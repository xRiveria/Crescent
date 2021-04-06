#include "VulkanPipeline.h"

namespace Crescent
{
	VulkanPipeline::VulkanPipeline(VkFormat& swapchainImageFormat) : m_SwapchainImageFormat(swapchainImageFormat)
	{
		CreateRenderPass();
		CreateGraphicsPipeline();
	}

	void VulkanPipeline::CreateRenderPass()
	{
		/*
			We need to tell Vulkan about the framebuffer attachments that will be used while rendering. We need to specify how many color and depth buffers there will be,
			how many samples to use for each of them and how their contents should be handled throughout the rendering operations. All of this information is wrapped in a
			render pass object, for which we will use this function to populate. In our case, we will only need a single color buffer attachment represented by one of the 
			images from the swapchain.
		*/
		VkAttachmentDescription attachmentInfo{};
		attachmentInfo.format = m_SwapchainImageFormat; //The format of the color attachment should match the format of the swapchain images.
		attachmentInfo.samples = VK_SAMPLE_COUNT_1_BIT; //Since we're not doing anything with multisampling yet, we will stick with 2 sample.
	}

	void VulkanPipeline::CreateGraphicsPipeline()
	{

	}
}