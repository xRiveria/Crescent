#include "VulkanPipeline.h"
#include "VulkanUtilities.h"
#include <array>

namespace Crescent
{
	VulkanPipeline::VulkanPipeline(const VkFormat& swapchainImageFormat, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice) :
		m_SwapchainImageFormat(swapchainImageFormat), m_PhysicalDevice(physicalDevice), m_LogicalDevice(logicalDevice)
	{
		CreateRenderPass();
		CreateGraphicsPipeline();
	}

	void VulkanPipeline::CreateRenderPass()
	{
		/*
			When a GPU renders a scene, it is comfigured with one or more render targets, or framebuffer attachments in Khronos terminology. The size and format of the
			attachment determine how graphics work is configured across the parallelism avaliable on all modern GPUs. Fore xample, on a tile-based renderer, the set of
			attachments is used to determine the way the image is divided into tiles. In Vulkan, a render pass is the set of attachments, the way they are used and
			the rendering work that is performed using them. In a traditional API, a change to a new render pass might correspond to binding a new framebuffer.

			We need to tell Vulkan about the framebuffer attachments that will be used while rendering. We need to specify how many color and depth buffers there will be,
			how many samples to use for each of them and how their contents should be handled throughout the rendering operations. All of this information is wrapped in a
			render pass object, for which we will use this function to populate. In our case, we will only need a single color buffer attachment represented by one of the 
			images from the swapchain.
		*/
		VkAttachmentDescription colorAttachmentInfo{};
		colorAttachmentInfo.format = m_SwapchainImageFormat; //The format of the color attachment should match the format of the swapchain images.
		colorAttachmentInfo.samples = VK_SAMPLE_COUNT_1_BIT; //Since we're not doing anything with multisampling yet, we will stick with 2 sample.
		/*
			The loadOp and storeOp determine what to do with the data in the attachment before rendering and after rendering. We have the following choices for loadOp:

			- VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment.
			- VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start.
			- VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined, we don't care about them.

			In our case, we're going to use the clear operation to clear the framebuffer to black before drawing a new frame. There are only two possibilities for storeOp:

			- VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later.
			- VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation.

			As we're only interested in seeing the rendered triangle on the screen, we're going with the store operation.
		*/
		colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		//The loadOp and storeOp apply to color and depth data, and stencilLoadOp/stencilStoreOp apply to stencil data. For now, we won't be doing anyting with the stencil buffer, so they are irrelevant for now.
		colorAttachmentInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		/*
			Textures and framebuffers in Vulkan are represented by VkImage objects with a certain pixel format, however the layout of the pixels in memory can change
			based on what you're trying to do with the image. The most important thing to know right now is that images need to be transitioned to certain layouts
			that are suitable for the operation they're going to be involved in next. Some of the most common layouts are:

			- VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attschment.
			- VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swapchain.
			- VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation.

			The initialLayout specifies which layout the image will have before the render pass begins. The finalLayout specifies the layout to automatically transition
			to when the render pass finishes. Using VK_IMAGE_LAYOUT_UNDEFINED for the initialLayout means that we don't care what the previous layout the image was in. The
			caveat of this special value is that the contents of the image are not guarenteed to be preserved, but that doesn't matter since we're going to clear it anyway.
			We want the image to be ready for presentation using the swapchain after rendering, which is why we use the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as finalLayout.
		*/
		colorAttachmentInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;



		/*
			A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations that depend on the contents of framebuffers in previous
			passes, for example a sequence of post-processing effects that are applied one after another. If you group these rendering operations into one render pass,
			then Vulkan will be able to reorder the operations and conserve memory bandwidth for possibly better performance. We can stick to a single subpass for now.
			Every subpass references one or more of the attachments that we've described using the structure in the previous sections. These references are themselves
			VkAttachmentReference structs that look like this:
		*/
		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0; //This parameter specifies which attachment to reference by its index in the attachment descriptions array.
		/*
			The layout parameter specifies which layout we would like the attachment to have during a subpass that uses this reference. Vulkan will automatically transition
			the attachment to the layout when the subpass is started. We intend to use the attachment to function as a color buffer and the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			layout will give us the best performance.
		*/
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachmentInfo{};
		depthAttachmentInfo.format = FindDepthFormat(*m_PhysicalDevice); //The format should be the same as the depth image itself.
		depthAttachmentInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Clears the values to a constant at the start.
		depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //Undefined after rendering operations.
		depthAttachmentInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //Existing contents are undefined, we don't care about them.
		depthAttachmentInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachmentInfo.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentReference{};
		depthAttachmentReference.attachment = 1; //Index 1 following the color attachment.
		depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//The subpass is described using a VkSubpassDescription structure.
		VkSubpassDescription subpassInfo{};
		subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //As compute subpasses may become avaliable, we have to be explicit about this being a graphics subpass.
		/*
			The index of the attachment in this array is directly referenced from the fragment shader with the "layout(location = 0) out vec4 outColor" directive. The
			following other types of attachments can be referenced by a subpass:
			- pInputAttachments: Attachments that are read by a shader.
			- pResolveAttachments: Attachments used for multisampling color attachments.
			- pDepthStencilAttachment: Attachment for depth and stencil data.
			- pPreserveAttachments: Attachments that are not used for this subpass, but for which the data must be preserved.
		*/
		subpassInfo.colorAttachmentCount = 1;
		subpassInfo.pColorAttachments = &colorAttachmentReference;
		subpassInfo.pDepthStencilAttachment = &depthAttachmentReference;
		/*
			Remember that the subpasses in a render pass automatically take care of image layout transitions. These transitions are controlled by subpass
			dependencies, which specify memory and exeuction dependencies between subpasses. We have only a single subpass right now, but the operations right
			before and right after this subpass also count as implicit "subpasses". There are two built-in dependencies that take care of the transition at the
			start of the render pass and at the end of the render pass, but the former does not occur at the right time. It assumes that the transition occurs
			at the start of the pipeline, but we haven't acquired the image yet at that point. There are two ways to deal with this problem.

			We could change the waitStages for the m_ImageAvaliableSemaphore to VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT to ensure that the render passes don't begin until the image
			is avaliable, or we can make the render pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage.

			Lets go with the second option here as it is a good excuse to have a look at subpass dependencies and how they work. Subpass dependencies are specified in
			VkSubpassDependency structs.
		*/

		VkSubpassDependency dependencyInfo{};
		/*
			The first two fields specify the indices of the dependency and the dependent subpass. The special value VK_SUBPASS_EXTERNAL refers to the implict subpass
			before or after the render pass depending on whether it is specified in srcSubpass or dstSubpass. The index 0 refers to our subpass, which is the first and
			only one. The dstSubpass must always be higher than srcSubpass to prevent cylces in dependency graph (unless one of the subpasses is VK_SUBPASS_EXTERNAL).

			We need to extend our subpass dependencies to make sure that there is no conflict between the transitioning of the depth image and it being cleared as part
			of its load operation. The depth image is first accessed in the early fragment test pipeline stage and because we have a load operation that clears, we should
			specify the access mask for writes.
		*/
		dependencyInfo.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencyInfo.dstSubpass = 0;
		/*
			The next two fields specify the operations to wait on and the stages in which these operations occur. We need to wait for the swapchain to finish reading
			from the image before we can access it. This can be accomplished by waiting on the color attachment output stage itself.
		*/
		dependencyInfo.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencyInfo.srcAccessMask = 0;
		/*
			The operations that should wait on this are in the color attachment stage and involves the writing of the color attachment. These settings will prevent the
			transition from happening until its actually necessary (and allowed): when we want to start writing colors to it.
		*/
		dependencyInfo.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencyInfo.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		
		/*
			Note that as the attachment and a basic subpass referencing it have been described, we can create the render pass itself. The render pass object can be
			created by filling in the VkRenderPassCreateInfo structure with an array of attachments and subpasses. The VkAttachmentReference object references
			attachments using the indices of this array.
		*/
		std::array<VkAttachmentDescription, 2> attachments = { colorAttachmentInfo, depthAttachmentInfo };

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassInfo;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependencyInfo;

		if (vkCreateRenderPass(*m_LogicalDevice, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Render Pass.");
		}
		else
		{
			std::cout << "Successfully created Render Pass.";
		}
	}

	void VulkanPipeline::CreateGraphicsPipeline()
	{

	}
}