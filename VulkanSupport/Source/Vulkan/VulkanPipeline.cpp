#include "VulkanPipeline.h"
#include "VulkanUtilities.h"
#include "VulkanShader.h"
#include <array>

namespace Crescent
{
	VulkanPipeline::VulkanPipeline(const VkFormat& swapchainImageFormat, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkExtent2D* swapchainExtent, VkDescriptorSetLayout* descriptorSetLayout) :
		m_SwapchainImageFormat(swapchainImageFormat), m_PhysicalDevice(physicalDevice), m_LogicalDevice(logicalDevice), m_SwapchainExtent(swapchainExtent), m_DescriptorSetLayout(descriptorSetLayout)
	{
		CreateRenderPass();
		CreateGraphicsPipeline();
	}

	void VulkanPipeline::DestroyPipelineInstance()
	{
		vkDestroyPipeline(*m_LogicalDevice, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(*m_LogicalDevice, m_PipelineLayout, nullptr);
		vkDestroyRenderPass(*m_LogicalDevice, m_RenderPass, nullptr);
	}

	void VulkanPipeline::CreateRenderPass()
	{
		/*
			When a GPU renders a scene, it is comfigured with one or more render targets, or framebuffer attachments in Khronos terminology. The size and format of the
			attachment determine how graphics work is configured across the parallelism avaliable on all modern GPUs. Fore example, on a tile-based renderer, the set of
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
			throw std::runtime_error("Failed to create Render Pass.\n");
		}
		else
		{
			std::cout << "Successfully created Render Pass.\n";
		}
	}

	void VulkanPipeline::CreateGraphicsPipeline()
	{
		VulkanShader vertexShader("Resources/Shaders/vertex.spv", VK_SHADER_STAGE_VERTEX_BIT, m_LogicalDevice);
		VulkanShader fragmentShader("Resources/Shaders/fragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_LogicalDevice);

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShader.RetrieveShaderStageInfo(), fragmentShader.RetrieveShaderStageInfo() };

		/*
			The older graphics APIs provided default state for most of the stages of the graphics pipeline. In Vulkan, you have to be explict about everything, from
			viewPort size to color blending functions. We will thus have to create each of these ourselves.

			The VkPipelineVertexInputStateCreateInfo structure describes the format of the vertex data that will be passed to the vertex shader. It describes this in roughly two ways:
			- Bindings: Spacing between data and whether data is per-vertex or per-instance (Geometry Instance, where multiple copies of the same mesh are rendered at once in a scene).
			- Attribute Descriptions: Type of the attributes passed to the vertex shader, which binding to load them from and at which offset.
		*/
		VkVertexInputBindingDescription bindingDescription = Vertex::RetrieveBindingDescription();
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Vertex::RetrieveAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; //This points to an array of structs the describe the aforementioned details for loading vertex data.

		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); //This points to an array of structs that describe the aforementioned details for loading vertex data.

		/*
			The VkPipelineInputAssemblyStateCreateInfo struct describes two things: what kind of geometry will be drawn from the vertices and if primitive restart should be enabled.
			The format is specified in the "topology" member and can have values like:

			- VK_PRIMITIVE_TOPOLOGY_POINT_LIST: Points from vertices.
			- VK_PRIMITIVE_TOPOLOGY_LINE_LIST: Line from every 2 vertices without reuse.
			- VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: The end vertex of every line is used as start vertex for the next line.
			- VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: Triangle from every 3 vertices without reuse.
			- VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: The second and third vertex of every triangle are used as the first two vertices of the next triangle.

			Normally, the vertices are loaded from the vertex buffer by index in sequential order, but with an element buffer, you can specify the indices to use yourself. This
			allows you to perform optimizations like reusing vertices. If you set the primitiveRestartEnable to VK_TRUE, then its possible to break up lines and triangles
			in the _STRIP topology modes by using a special index of 0xFFFF or 0xFFFFFFF.
		*/

		//As we intend to draw triangles throughout this tutorial, we will stick to the following data for the structure.
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		/*
			A viewport basically describes the region of the framebuffer that the output will be rendered to. This will almost always be (0, 0) to (Width, Height).
			Remember the size of the swapchain and its images may differ from the width/height of the window. The swapchain images will be used as framebuffers later on,
			so we should stick to their size.
		*/
		VkViewport viewportInfo{};
		viewportInfo.x = 0.0f;
		viewportInfo.y = 0.0f;
		viewportInfo.width = (float)m_SwapchainExtent->width;
		viewportInfo.height = (float)m_SwapchainExtent->height;
		viewportInfo.minDepth = 0.0f; //The minDepth and maxDepth values specify the range of depth values to use for the framebuffer.
		viewportInfo.maxDepth = 1.0f; //These values must be within the [0.0f, 1.0f] range, but minDepth may be higher than maxDepth. If you aren't doing anything special, then you should stick to the standard values of 0.0f and 1.0f.

		/*
			While viewports define the transformation from the image to the framebuffer, scissor rectangles define in which regions pixels will actually be stored. Any pixel
			outside the scissor rectangles will be discarded by the rasterizer. They function like a filter rather than a transformation. A we wish to draw to the entire framebuffer,
			we will specify a scissor rectangle that covers it entirely.
		*/
		VkRect2D scissorInfo{};
		scissorInfo.offset = { 0, 0 };
		scissorInfo.extent = *m_SwapchainExtent;

		/*
			Now, this viewport and scissor rectangle need to be combined into a viewport state using the VkPipelineViewportStateCreateInfo struct. It is possible to use
			multiple viewports and scissor rectangles on some graphics cards, so its members reference an array of them. Using multiple requires enabling a GPU feature.
		*/
		VkPipelineViewportStateCreateInfo viewportStateInfo{};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.pViewports = &viewportInfo;
		viewportStateInfo.scissorCount = 1;
		viewportStateInfo.pScissors = &scissorInfo;

		/*
			The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into fragments to be colored by the fragment shader. It also
			performs depth testing, face culling and the scissor test, and it can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering).
			All this is configured using the VkPipelineRasterizationStateCreateInfo structure.
		*/
		VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
		rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		/*
			If this is true, then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them. This is useful in special cases like
			shadow maps. This requires a GPU feature as well.
		*/
		rasterizerInfo.depthClampEnable = VK_FALSE;
		/*
			The polygonMode determines how fragments are generated for geometry. The following modes are avaliable and using any other mode other than fill requires enabling a GPU feature.
			- VK_POLYGON_MODE_FILL: Fills the area of the polygon with fragments.
			- VK_POLYGON_MODE_LINE: Polygon edges are drawn as lines.
			- VK_POLYGON_MODE_POINT: Polygon vertices are drawn as points.
		*/
		rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
		/*
			The lineWidth member is straightforward. It describes the thickness of lines in terms of number of fragments. The maximum line width that is supported depends on the
			hardware and any line thicker than 1.0f requires you to enable the wideLines GPU feature.
		*/
		rasterizerInfo.lineWidth = 1.0f;
		/*
			The cullMode variable determines the type of face culling to use. You can disable culling, cull the front faces, cull the back faces or both. The frontFace variable
			describes the vertex order for faces to be considered front-facing and can be clockwise or counter-clockwise.
		*/
		rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		//The rasterizer can alter ther depth values by adding a constant value or biasing them based on the fragment's slope. This is sometimes used for shadow mapping, but we won't be using it.
		rasterizerInfo.depthBiasEnable = VK_FALSE;
		rasterizerInfo.depthBiasConstantFactor = 0.0f; //Optional
		rasterizerInfo.depthBiasClamp = 0.0f; //Optional
		rasterizerInfo.depthBiasSlopeFactor = 0.0f; //Optional

		/*
			The VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the ways to perform antialiasing. It works by combining the fragment
			shader results of multiple polygons that rasterize to the same pixel. This mainly occurs along edges, which is also where the most noticeable aliasing artifacts
			occur. Because it doesn't need to run the fragment shader multiple times if only one polygon maps to a pixel, it is significantly less expensive than
			simply rendering to a higher resolution and then downscaling. Enabing it requires enabling a GPU feature.
		*/
		VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
		multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingInfo.sampleShadingEnable = VK_FALSE;
		multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingInfo.minSampleShading = 1.0f;
		multisamplingInfo.pSampleMask = nullptr;
		multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
		multisamplingInfo.alphaToOneEnable = VK_FALSE;

		/*
			After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer. This transformation is known as color blending
			and there are two ways to do it:

			- Mix the old and new value to produce a final color.
			- Combine the old and new value using a bitwise operation.

			There are two types of structs as well to configure color blending. The first struct, VkPipelineColorBlendAttachmentState contains the configuration per attached
			framebuffer and the second struct VkPipelineColorBlendingStateCreateInfo contains the global color blending settings. In our case, we only have 1 framebuffer.
		*/
		VkPipelineColorBlendAttachmentState colorBlendInfo{};
		colorBlendInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendInfo.blendEnable = VK_FALSE;
		colorBlendInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendInfo.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendInfo.alphaBlendOp = VK_BLEND_OP_ADD;

		//The second struct references an array of structs for all of the framebuffers and allows you to set blend constants that you can use as blend factors in the aforementioned calculations.
		VkPipelineColorBlendStateCreateInfo globalBlendInfo{};
		globalBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		/*
			Set this to true if you wish to use the second method of blending (bitwise combination). This will automatically disable the first method, as if you have set
			blendEnable to VK_FALSE for every attached framebuffer. Of course, it is possible to disable both modes as we've done here, in which case the fragment colors will be written
			to the framebuffer unmodified.
		*/
		globalBlendInfo.logicOpEnable = VK_FALSE;
		globalBlendInfo.logicOp = VK_LOGIC_OP_COPY; //This is where the bitwise operation can be specified.
		globalBlendInfo.attachmentCount = 1; //We only have 1 framebuffer.
		globalBlendInfo.pAttachments = &colorBlendInfo;
		globalBlendInfo.blendConstants[0] = 0.0f;
		globalBlendInfo.blendConstants[1] = 0.0f;
		globalBlendInfo.blendConstants[2] = 0.0f;
		globalBlendInfo.blendConstants[3] = 0.0f;

		/*
			You can use uniform values in shaders, which are global similar to dynamic state variables that can be changed at drawing time to alter the behavior of your shaders
			without having to recreate them. They are commonly used to pass the transformation matrix to the vertex shader, or to create texture samplers in the fragment shader.
			These uniform values need to be specified during pipeline creation by creating a vkPipelineLayout object. If we won't be using them, we are still
			required to create empty pipeline layout.
		*/
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = m_DescriptorSetLayout; //We need to specify the descriptor set layout during pipeline creation to tell Vulkan which descriptors the shaders will be using.
		pipelineLayoutInfo.pushConstantRangeCount = 0; //Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; //Optional. Push Constants are another way of passing dynamic values to shaders.

		if (vkCreatePipelineLayout(*m_LogicalDevice, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Pipeline Layout.\n");
		}
		else
		{
			std::cout << "Successfully created Pipeline Layout. \n";
		}

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilInfo.depthTestEnable = VK_TRUE; //Specifies if the depth of new fragments should be compared to the depth buffer to see if they should be discarded.
		depthStencilInfo.depthWriteEnable = VK_TRUE; //Specifies if the depth of new fragments that pass the depth test should actually be written to the depth buffer.
		//Specifies the comparison that is performed to keep or discard fragments. We're sticking to the convention of lower depth = closer, so the depth of new fragments should be less.
		//Essentially, we're saying that whichever is closer is kept.
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS; 
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE; //These are used for optional depth bound tests. This allows you to only keep fragments that fall within the specified depth range.
		depthStencilInfo.minDepthBounds = 0.0f;
		depthStencilInfo.maxDepthBounds = 1.0f;
		depthStencilInfo.stencilTestEnable = VK_FALSE; //For stencil buffer operations. If we want to use these, ensure that the format of the depth/stencil image contains a stencil component.
		depthStencilInfo.front = {};
		depthStencilInfo.back = {};

		//Finally, we have our graphics pipeline.
		VkGraphicsPipelineCreateInfo pipelineCreationInfo{};
		pipelineCreationInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreationInfo.pStages = shaderStages;
		//Then, we start by referencing the array of VkPipelineShaderStageCreateInfo structs.
		pipelineCreationInfo.stageCount = 2;
		pipelineCreationInfo.pStages = shaderStages;
		//Then, we reference all of the structure describing the fixed-function stage.
		pipelineCreationInfo.pVertexInputState = &vertexInputInfo;
		pipelineCreationInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineCreationInfo.pViewportState = &viewportStateInfo;
		pipelineCreationInfo.pRasterizationState = &rasterizerInfo;
		pipelineCreationInfo.pMultisampleState = &multisamplingInfo;
		pipelineCreationInfo.pDepthStencilState = &depthStencilInfo;
		pipelineCreationInfo.pColorBlendState = &globalBlendInfo;
		pipelineCreationInfo.pDynamicState = nullptr;
		//After that comes the pipeline layout, which is a Vulkan handle rather than a struct pointer.
		pipelineCreationInfo.layout = m_PipelineLayout;
		/*
			Finally, we have a reference to the render pass and the index of the subpass where this graphics pipeline will be used. It is also possible to use other render passes
			with this pipeline instead of this specific instance, but they have to be compatible with the renderPass parameter. The requirements for compatibility are described
			in the specification, 
		*/
		pipelineCreationInfo.renderPass = m_RenderPass;
		pipelineCreationInfo.subpass = 0;

		/*
			There are two more parameters: basePipelineHandle and basePipelineIndex. Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline.
			The idea of pipeline derivatives is that it is less expensive to set up pipeline when they have much functionality in common with an existing pipeline and switching
			between pipelines from the same parent can also be done much quicker. You can either specify the handle of an existing pipeline with basePipelineHandle or reference
			another pipeline that is about to be created by index with basePipelineIndex. Right now, there is only a single pipeline, so we will specify a null handle and an invalid
			index. These values are only used if the VK_PIPELINE_CREATE_DERIVATE_BIT flag is also specified in the flags fiedld of VkGraphicsPipelineCreateInfo.

			The vkCreateGraphicsPipelines function actually has more parameters than the usual object creation functions in Vulkan. It is designed to take multiple 
			VkGraphicsPipelineCreateInfo objects and create multiple VkPipeline objects in a single call. The second parameter, in which we've passed VK_NULL_HANDLE
			as argument, references an optional VkPipelineCache object. This makes it possible to significantly speed up pipeline creation at a later time. We will get into
			this eventually.
		*/
		if (vkCreateGraphicsPipelines(*m_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreationInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline.\n");
		}
		else
		{
			std::cout << "Successfully created Graphics Pipeline.\n";
		}

		vertexShader.DestroyShaderInstance();
		fragmentShader.DestroyShaderInstance();
	}
}