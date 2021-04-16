#include "VulkanRenderer.h"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <chrono>
#include "../Source/Vulkan/VulkanUtilities.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Crescent
{
	const int g_MaxFramesInFlight = 2;
	size_t g_CurrentFrameIndex = 0;

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		VulkanRenderer* renderer = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
		renderer->m_FramebufferResized = true;
	}

	VulkanRenderer::VulkanRenderer(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion, const bool& validationLayersEnabled)
		: m_ValidationLayersEnabled(validationLayersEnabled)
	{
		m_Window = std::make_shared<Window>(1280, 1080, "Vulkan Demo");
		glfwSetWindowUserPointer(m_Window->RetrieveWindow(), this);
		glfwSetWindowUserPointer(m_Window->RetrieveWindow(), FramebufferResizeCallback);
		m_DebugMessenger = std::make_shared<VulkanDebug>(); //To be created first before the instance so as to allow for callbacks during instance creation.

		CreateVulkanInstance(applicationName, engineName, applicationMainVersion, applicationSubVersion);
		if (validationLayersEnabled)
		{
			m_DebugMessenger->SetupDebugMessenger(&m_VulkanInstance);
		}
		CreatePresentationSurface();
		m_Devices = std::make_shared<VulkanDevice>(&m_VulkanInstance, &m_Surface);
		m_Swapchain = std::make_shared<VulkanSwapchain>(m_Devices->RetrievePhysicalDevice(), m_Devices->RetrieveLogicalDevice(), &m_Surface, m_Window->RetrieveWindow());
		m_DescriptorLayout = std::make_shared<VulkanDescriptorLayout>(m_Devices->RetrieveLogicalDevice());
		m_Pipeline = std::make_shared<VulkanPipeline>(m_Swapchain->RetrieveSwapchainImageFormat(), m_Devices->RetrievePhysicalDevice(), m_Devices->RetrieveLogicalDevice(), m_Swapchain->RetrieveSwapchainExtent(), m_DescriptorLayout->RetrieveDescriptorSetLayout(), *m_Devices->RetrieveMSAASampleCount());
		m_CommandPool = std::make_shared<VulkanCommandPool>(m_Devices->RetrieveLogicalDevice(), m_Devices->RetrievePhysicalDevice(), &m_Surface);
		m_Swapchain->CreateMultisampledColorBufferResources(*m_Devices->RetrieveMSAASampleCount());
		m_Swapchain->CreateDepthBufferResources(m_CommandPool->RetrieveCommandPool(), m_Devices->RetrieveGraphicsQueue(), *m_Devices->RetrieveMSAASampleCount());
		m_Swapchain->CreateFramebuffers(m_Pipeline->RetrieveRenderPass());

		m_ModelTexture = std::make_shared<VulkanTexture>("Resources/Textures/viking_room.png", m_Devices->RetrieveLogicalDevice(), m_Devices->RetrievePhysicalDevice(), VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_CommandPool->RetrieveCommandPool(), m_Devices->RetrieveGraphicsQueue());
		m_Model = std::make_shared<VulkanResource>("Resources/Models/viking_room.obj");
		m_VertexBuffer = std::make_shared<VulkanVertexBuffer>(m_Model, m_Devices->RetrievePhysicalDevice(), m_Devices->RetrieveLogicalDevice(), m_Devices->RetrieveGraphicsQueue(), m_CommandPool->RetrieveCommandPool());
		m_IndexBuffer = std::make_shared<VulkanIndexBuffer>(m_Model, m_Devices->RetrievePhysicalDevice(), m_Devices->RetrieveLogicalDevice(), m_Devices->RetrieveGraphicsQueue(), m_CommandPool->RetrieveCommandPool());
		m_Swapchain->CreateUniformBuffers();
		m_DescriptorPool = std::make_shared<VulkanDescriptorPool>(m_Swapchain->RetrieveSwapchainImages(), m_Devices->RetrieveLogicalDevice());
		m_Swapchain->CreateDescriptorSets(m_ModelTexture, m_DescriptorLayout->RetrieveDescriptorSetLayout(), m_DescriptorPool->RetrieveDescriptorPool());

		CreateSynchronizationObjects();
		CreateCommandBuffers();
	}

	VulkanRenderer::~VulkanRenderer()
	{

	}

	//Retrieves the names of extensions required by our Vulkan application.
	static std::vector<const char*> RetrieveRequiredVulkanExtensions(const bool& validationLayersEnabled)
	{
		//Vulkan is platform agnostic. Thus, extensions are needed to interface with the window system. GLFW has a handy function that returns the extension(s) it needs to do that.
		uint32_t glfwExtensionsCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount); //Returns an array of Vulkan instance extensions and stores the count in a provided buffer. 

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount); //Creates a vector with data of first parameter to the end of length in second parameter.
		for (const auto& extension : extensions)
		{
			std::cout << "GLFW Extension Required: " << extension << "\n";
		}

		if (validationLayersEnabled)
		{
			/*
				While certain extensions (such as the GLFW ones) are always required, the debug messenger extension is conditionally added. Validation layers will allow Vulkan
				to print debug messages to the standard output by default, which we can overwrite with our own explict callback. This will allow us to decide what kind of
				messages we want to see as not all of them are necessarily important/fatal enough.

				VK_EXT_debug_utils will allow us to setup a debug messenger to handle debug messages and their associated details.
			*/
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); //Equals the literal string of "VK_EXT_debug_utils".
		}

		return extensions;
	}

	void VulkanRenderer::CreateVulkanInstance(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion)
	{
		//Validation layers are optional components that hook into Vulkan function calls to apply operations such as parameter checking, Vulkan object tracking, call logging etc.
		if (m_ValidationLayersEnabled && !m_DebugMessenger->QueryValidationLayersSupport())
		{
			throw::std::runtime_error("Validation Layers Requested, but not avaliable!");
		}

		//Passes information on our application to the implementation.
		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = applicationName.c_str();
		applicationInfo.applicationVersion = VK_MAKE_VERSION(applicationMainVersion, applicationSubVersion, 0);
		applicationInfo.pEngineName = engineName.c_str();
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion = VK_API_VERSION_1_0;

		//Passes information on which global extensions and validation layers we wish to use, as well as our application information to the Vulkan drivers.
		VkInstanceCreateInfo creationInfo{};
		creationInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		creationInfo.pApplicationInfo = &applicationInfo;

		//Extensions Information.
		std::vector<const char*> requiredExtensions = RetrieveRequiredVulkanExtensions(m_ValidationLayersEnabled);
		creationInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		creationInfo.ppEnabledExtensionNames = requiredExtensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreationInfo; //This is placed outside the if statement so that it is not destroyed before the vkCreateInstance call below.
		if (m_ValidationLayersEnabled)
		{
			creationInfo.enabledLayerCount = static_cast<uint32_t>(m_DebugMessenger->m_ValidationLayers.size());
			creationInfo.ppEnabledLayerNames = m_DebugMessenger->m_ValidationLayers.data(); //Determines the global validation layers based on our specifications.
		/*
			Even though we may have added debugging with validation layers, we're not quite convering everything. The vkCreateDebugUtilsMessengerEXT call requires a valid
			instance to have been created and vkDestroyDebugUtilsMessengerEXT must be called before the instance is destroyed. This leaves us unable to debug
			issues with the vkCreateInstance and vkDestroyInstance calls.

			However, you will see in the extension documentation that there is a way to create seperate debug utils messenger specifically for those 2 function calls. It
			requires you to simply pass a pointer to a VkDebugUtilsMessengerCreateInfoEXT struct in the pNext extension field of VkInstanceCreateInfo. By creating an
			additional debug messenger this way, it will automatically be used during vkCreateInstance and vkDestroyInstance and cleaned up after that.
		*/
			m_DebugMessenger->PopulateDebugMessengerCreationInfo(debugCreationInfo);
			creationInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreationInfo;
		}
		else
		{
			creationInfo.enabledLayerCount = 0;
			creationInfo.pNext = nullptr;
		}
		
		if (vkCreateInstance(&creationInfo, nullptr, &m_VulkanInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan instance.");
		}
		else
		{
			std::cout << "Successfully created Vulkan Instance.\n";
		}
	}

	void VulkanRenderer::CreatePresentationSurface()
	{
		/*
			The window surface needs to be created after instance creation as it can actually influence the physical device selection. As Vulkan is a platform agnostic API,
			it cannot interface directly with the windowing system on its own. Thus, we need to establish this connection to present results to the screen. We will thus be
			using the WSI (Window System Integration) extensions. The first one is VK_KHR_surface. It exposes a VkSurfaceKHR object that represents an abstract type of surface
			to present rendered images to. The surface in our program will be backed by the window that we have already opened with GLFW. 

			Note that the VK_KHR_surface extension is returned by glfwGetRequiredInstanceExtensions (thus instance level) that we have already enabled alongsided some other WSI
			extensions. Note that window surfaces are entirely optional in Vulkan. If you just need off-screen rendering, Vulkan allows you to do that unlike OpenGL where a window is at least needed.
		
			Although the VkSurfaceKHR object and its usage is platform agnostic, its creation isn't as it depends on window system details. For example, it requires the HWND
			and HMODULES handles on Windows. There is also a platform-specific addition to the extension, which on Windows is called VK_KHR_win32_surface, once again included in
			the list from glfwGetRequiredInstanceExtensions. Lets demonstrate how this platform specific extension can be used to create a surface on Windows. However, we won't
			be using its method. GLFW has a method for us which does this automatically instead.

			VkWin32SurfaceCreateInfoKHR creationInfo{};
			creationInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			creationInfo.hwnd = glfwGetWin32Window(m_Window); //Retrieves the raw HWND from the GLFW window object.
			creationInfo.hinstance = GetModuleHandle(nullptr); //Returns the HINSTANCE handles of the current process.

			if (vkCreateWin32SurfaceKHR(m_VulkanInstance, &creationInfo, nullptr, &m_Surface) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create Window Surface.");
			}
		*/

		//GLFW provides us with an implementation that does all of the above, with a different implementation avaliable for each platform.
		if (glfwCreateWindowSurface(m_VulkanInstance, m_Window->RetrieveWindow(), nullptr, &m_Surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Window Surface.");
		}
		else
		{
			std::cout << "Successfully created Vulkan Window surface.\n";
		}
	}

	void VulkanRenderer::UpdateUniformBuffers(uint32_t currentImage)
	{
		/*
			This function will generate a new transformation every frame to make the geometry spin around. We will make our geometry rotate 90 degrees per second regardless of 
			framerate. We will now calculate the time in seconds since rendering has started with floating point accuracy. 
		*/
		static auto startTime = std::chrono::high_resolution_clock::now(); //Static so as the function is called repeatedly, this won't get redefined and overwritten.
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject uniformBufferObject{};
		//Rotation on the Z axis of 90 degrees per second.
		uniformBufferObject.m_ModelMatrix = glm::rotate(glm::mat4(1.0f), (time / 3) * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//The glm::LookAt function takes the eye position, center position and up axis. This is usually a camera, but we don't have this at the moment.
		uniformBufferObject.m_ViewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), (glm::vec3(0.0f, 0.0f, 1.0f)));
		//The perspective projection will be a 45 degree vertical field-of-view.
		uniformBufferObject.m_ProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)m_Swapchain->RetrieveSwapchainExtent()->width / (float)m_Swapchain->RetrieveSwapchainExtent()->height, 0.1f, 10.0f);
		/*
			As GLM was originally was designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. The easiest way to compensate for that is to flip the sign on
			the scaling factor of the Y axis in the projection matrix. If you don't do that, the image will be rendered upside down.
		*/
		uniformBufferObject.m_ProjectionMatrix[1][1] *= -1;

		void* data;
		vkMapMemory(*m_Devices->RetrieveLogicalDevice(), m_Swapchain->m_UniformBuffersMemory[currentImage], 0, sizeof(uniformBufferObject), 0, &data);
		memcpy(data, &uniformBufferObject, sizeof(uniformBufferObject));
		vkUnmapMemory(*m_Devices->RetrieveLogicalDevice(), m_Swapchain->m_UniformBuffersMemory[currentImage]);
	}

	void VulkanRenderer::CreateSynchronizationObjects()
	{
		//We will need one semaphore to signal than an image has been acquired and is ready for rendering, and another one to signal that rendering has finished and presentation can happen.
		m_ImageAvaliableSemaphores.resize(g_MaxFramesInFlight);
		m_RenderFinishedSemaphores.resize(g_MaxFramesInFlight);
		m_InFlightFences.resize(g_MaxFramesInFlight);
		m_ImagesInFlightFences.resize(m_Swapchain->RetrieveSwapchainImages()->size(), VK_NULL_HANDLE); //Initially, not a single frame is using an image so we explictly initialize it to no fence.
	
		for (size_t i = 0; i < g_MaxFramesInFlight; i++)
		{
			m_ImageAvaliableSemaphores[i].CreateVulkanSemaphore(m_Devices->RetrieveLogicalDevice());
			m_RenderFinishedSemaphores[i].CreateVulkanSemaphore(m_Devices->RetrieveLogicalDevice());
			m_InFlightFences[i].CreateVulkanFence(m_Devices->RetrieveLogicalDevice());
		}
	}

	void VulkanRenderer::CreateCommandBuffers()
	{
		/*
			With the command pool created, we can now start allocating command buffers and recording drawing commands in them. Because one of the drawing commands involves 
			binding the right vkFramebuffer, we will actually have to record a command buffer for every image in the swapchain once again. Thus, we have to create a list of 
			vkCommandBuffer objects. These command buffers will automatically be freed once their command pool is destroyed, so we don't need an explict cleanup.

			Command buffers are allocated with the vkAllocateCommandBuffers function, which takes a VkCommandBufferAllocateInfo struct as parameter that specifies the command pool 
			and number of buffers to allocate.
		*/
		m_CommandBuffers.resize(m_Swapchain->m_SwapchainFramebuffers.size());

		VkCommandBufferAllocateInfo commandBufferInfo{};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.commandPool = *m_CommandPool->RetrieveCommandPool();
		/*
			The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
			- VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
			- VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from the primary command buffers.
			While we won't make use of secondary command buffer functionalities, you can imagine that it is helpful to reuse common operations from primary command buffers.
		*/
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		if (vkAllocateCommandBuffers(*m_Devices->RetrieveLogicalDevice(), &commandBufferInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate Command Buffers.\n");
		}
		else
		{
			std::cout << "Successfully allocated " << m_CommandBuffers.size() << " Command Buffers.\n";
		}

		/*
			We begin recording a command buffer by calling vkBeginCommandBuffer with a small VkCommandBufferBeginInfo struct as argument that specifies some details about the 
			usage of this specific command buffer.
		*/
		for (size_t i = 0; i < m_CommandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			/*
				The flags parameter specifies how we're going to use the command buffer. The following values are avaliable:
				- VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be re-recorded right after executing it once.
				- VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
				- VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also pending execution.

				None of these flags are applicable for us at the moment. The pInheritenceInfo parameter is only relevant for secondary command buffers. It specifies which
				state to inherit from the calling primary command buffers. If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implictly
				reset it. Its not possible to append commands to buffer at a later time.
			*/
			beginInfo.flags = 0; //Optional
			beginInfo.pInheritanceInfo = nullptr; //Optional

			if (vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to begin recording command buffer.\n");
			}
			else
			{
				std::cout << "Successfully recorded command buffer.\n";
			}

			//Drawing starts by beginning the render pass with vkCmdBeginRenderPass. The render pass is configured using some parameters in a VkRenderPassBeginInfo struct.
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			//The first parameters are the render pass itself and the attachments to bind. We created a framebuffer for each swapchain image that specifies it as color attachment.
			renderPassInfo.renderPass = *m_Pipeline->RetrieveRenderPass();
			renderPassInfo.framebuffer = m_Swapchain->m_SwapchainFramebuffers[i];
			/*
				The next two parameters define the size of the render area. The render area defines where shaders load and stores will take place. The pixels outside this region
				will have undefined values. It should match the size of the attachments for best performance.
			*/
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = *m_Swapchain->RetrieveSwapchainExtent();
			//The last two parameters define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR, which we used as load operation for the color attachment. Here, we define it to
			//be simply black with 100% opacity.
			std::array<VkClearValue, 2> clearColor;
			clearColor[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			/*
				The range of the depths in the depth buffer should be 0.0 to 1.0 in Vulkan, where 1.0 lies at the far view plane and 0.0 at the near view plane. The initial value
				at each point in the depth buffer should be the furthest possible depth, which is 1.0. Note that the order of clearValues should be identical to the order of your attachments.
			*/
			clearColor[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
			renderPassInfo.pClearValues = clearColor.data();

			/*
				The render pass can now begin. All of the functions that record commands can be recognized by their vkCmd prefix. They all return void, so there will be no error
				handling until we've finished recording. The first parameter for every command is always the command buffer to record the command to. The second parameter
				specifies the details of the render pass we've just provided. The final parameter controls how the drawing commands within the render passs will be provided.
				It can have one of two values:

				- VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
				- VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass contents will be executed from secondary command buffers.

				We will not be using secondary command buffers, so we will go with the first option.
			*/
			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			//We can now bind the graphics pipeline. The second parameter specifies if the pipeline object is a graphics or compute pipeline.
			vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *m_Pipeline->RetrievePipeline());

			/*
				We've now told Vulkan which operations to execute in the graphics pipeline and which attachment to use in the fragment shader, so all that remains is telling it to
				draw the triangle. The actual vkCmdDraw function is a bit anticlimatic, but its also so simple because of all of the information has been specified in advance. It
				has the following parameters, aside from the command buffer:

				- vertexCount: The number of vertices we have in whatever we're drawing.
				- instanceCount: Used for instanced rendering, use 1 if you're not doing that.
				- firstVertex: Used as an offset into the vertex buffer, defines the lowest point of gl_VertexIndex.
				- firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceVertex.
			*/
			VkBuffer vertexBuffers[] = { *m_VertexBuffer->RetrieveVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			/*
				The vkCmdBindVertexBuffers function is used to bind vertex buffers to bindings. The first two parameters, besides the command buffer, specifies the offset and 
				the number of bindings we're going to specify vertex buffers for. The last two parameters specify the array of vertex buffers to bind and the byte offsets 
				to read the vertex data from.
			*/
			vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);

			/*
				Using an index buffer requires us to first bind it, just like we did for the vertex buffer. The difference is that we can only have a single index buffer. Its 
				unfortunately not possible to use different indices for each vertex attribute, so we do still have to completely duplicate vertex data even if just one attribute 
				varies. An index buffer is bound with vkCmdBindIndexBuffer, which has the index buffer, a byte offset into it and the type of index data as parameters. As
				mentioned before, the possible types are VK_INDEX_TYPE_UINT16 and VK_INDEX_TYPE_UINT32.
			*/
			vkCmdBindIndexBuffer(m_CommandBuffers[i], *m_IndexBuffer->RetrieveIndexBuffer(), 0, VK_INDEX_TYPE_UINT32); //The data format should be the same as your vertices.

			/*
				We now need to bind the right descriptor set for each swapchain image to the descriptors in the shader with vkCmdBindDescriptorSets. Unlike vertex and index 
				buffers, descriptor sets are not unique to graphics pipelines. Therefore, we need to specify if we want to bind descriptor sets to the graphics or compute 
				pipeline. The next parameter is the layout that the descriptors are based on. The next three parameters specify the index of the first descriptor set, 
				the number of sets to bind, and the array of sets to bind.

				The last two parameters specify an array of offsets that are used for dynamic descriptors which we will look at in the future.
			*/
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *m_Pipeline->RetrievePipelineLayout(), 0, 1, &m_Swapchain->m_DescriptorSets[i], 0, nullptr);

			/*
				We now need to tell Vulkan to change the drawing command to use the Index Buffer.
			*/
			vkCmdDrawIndexed(m_CommandBuffers[i], static_cast<uint32_t>(m_Model->m_Indices.size()), 1, 0, 0, 0);

			//The render pass can be ended.
			vkCmdEndRenderPass(m_CommandBuffers[i]);
			//And we've finished recording the command buffer.
			if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record Command Buffer.\n");
			}
			else
			{
				std::cout << "Successfully recorded Command Buffer.\n";
			}
		}
	}

	void VulkanRenderer::DrawFrames()
	{
		while (!glfwWindowShouldClose(m_Window->RetrieveWindow()))
		{
			glfwPollEvents();

			/*
				Wait for the previous operations/frame to finish before continuing with drawing operations. The vkWaitForFences function takes an array of fences and waits 
				for either any or all of them to be signalled before returning. The VK_TRUE we pass in here signifies that we want to wait for all fences, but in the case of a 
				single one, it obviously doesn't really matter. Just like vkAcquireNextImageKHR, this function also takes a timeout. Unlike the semaphores, we manually need to 
				restore the fence to the unsignalled state by resetting it with the vkResetFences call.

				However, if you simply run the program, nothing will render. This is because by default, fences are created in unsignaled state. That means that vkWaitForFences 
				will wait forever if we haven't use the fence before. To solve that, we can change the fence creation to initialize it in signalled state as if we had rendered an initial
				frame that finished.
			*/
			vkWaitForFences(*m_Devices->RetrieveLogicalDevice(), 1, &m_InFlightFences[g_CurrentFrameIndex].m_Fence, VK_TRUE, UINT64_MAX);

			/*
				We will perform the following operations:
				1) Acquire an image from the swapchain.
				2) Execute the command buffer with that image as attachment in the framebuffer.
				3) Return the image to swapchain for presentation.

				Each of these events is set in motion using a single function call, but they are executed asynchronously. The function calls will return before the operations are 
				actually finished and the order of execution is also undefined. That is unfortunate, because each of the operations depend on the previous one finishing.

				There are two ways of synchronizing swapchain events: fences and semaphores. They're both objects that can be used to coordinate operations by having one operation 
				signal and another operation wait for a fence or semaphore to go from the unsignaled state to siognalled state. The difference is that the state of fences can be 
				accessed from your program using calls like vkWaitForFences and semaphores cannot be. Fences are mainly designed to synchronize your application itself with rendering 
				operations, whereas sempahores are used to sychronize operations within or across command queues. We want to synchronize the queue operations of drawing commands 
				and presentations, which makes semaphores the best fit.

				As mentioned before, the first thing we need to do here is to acquire an image from the swapchain. Recall that the swapchain is an extension feature, so we must use a function 
				with the vhKHR naming convention.
			*/
			uint32_t imageIndex;

			/*
				We have to figure out when swapchain recreation is necessary and call our RecreateSwapchain function accordingly. Luckily, Vulkan will usually just tell us that 
				the swapchain is no longer adequate during presentation. The vkAcquireNextImageKHR and vkQueuePresentKHR functions can return the following special values to indiciate 
				this:
				- VK_ERROR_OUT_OF_DATE_KHR: The swapchain has become incompatible with the surface and can no longer be used for rendering. Usually happens after a window resize.
				- VK_SUBOPTIMAL_KHR: The swapchain can still be used to successfully present to the surface, but the surface properties are no longer matched directly.

				The first two parameters of vkAcquireNextImageKHR are the logical device and the swapchain from which we wish to acquire an image. The third parameter specifies a timeout 
				in nanoseconds for an image to become avaliable. Using a maximum value of a 64 bit unsigned integer disables the timeout. The next two parameters specify synchronization 
				objects that are to be signalled when the presentation engine is finished using the image. The last parameter specifies a variable to output the index of the swapchain 
				image that has become avaliable. The index refers to the VkImage in our m_SwapchainTextures array. We're going to use that index to pick the right command buffer.
			*/

			VkResult result = vkAcquireNextImageKHR(*m_Devices->RetrieveLogicalDevice(), m_Swapchain->m_Swapchain, UINT64_MAX, *m_ImageAvaliableSemaphores[g_CurrentFrameIndex].RetrieveSemaphore(), VK_NULL_HANDLE, &imageIndex);

			/*
				If the swapchain turns out to be out of date when attempting to acquire an image, then it is no longer possible to present to it. Thus, we should recreate the swapchain 
				at once and try again in the next DrawFrames call. We should also decide to do so if the swapchain is suboptimal, but we've chosen to proceed anyway in that case as we've
				already acquired an image. Both VK_SUCCESS and VK_SUBOPTIMAL_KHR are considered "success" return codes. 
			*/
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				std::cout << "================================================= Swapchain out of date. Recreating... =================================================\n";
				RecreateSwapchain();
				return;
			}
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			{
				throw std::runtime_error("Failed to acquire Swapchain Image.\n");
			}

			//Now, we will wait on any previous frame that is using the image that we've just assigned for the new frame. Lets check if a previous frame is using this image (there is its fence to wait on).
			if (m_ImagesInFlightFences[imageIndex] != VK_NULL_HANDLE)
			{
				vkWaitForFences(*m_Devices->RetrieveLogicalDevice(), 1, &m_ImagesInFlightFences[imageIndex], VK_TRUE, UINT64_MAX);
			}
			
			//Once true, mark the image as now being in use by this frame.
			m_ImagesInFlightFences[imageIndex] = m_InFlightFences[g_CurrentFrameIndex].m_Fence;
			UpdateUniformBuffers(imageIndex);

			//Queue submission and synchronization is configured through parameters in the VkSubmitInfo structure.
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			/*
				The first three parameters specify which semaphores to wait on before execution begins and in which stage(s) of the pipeline to wait. We want to prevent from 
				writing colors to the image until its avaliable, so we're specifying the start of the graphics pipeline that write to the color attachment. That means that 
				theoreotically, the implementation can already start exeucting our vertex shader and such while the image is not yet avaliable. We don't want that to happen.
				Note that each entry in the waitStages array corresponds to the semaphore with the same index in pWaitSemaphores.
			*/

			VkSemaphore waitSemaphores[] = { *m_ImageAvaliableSemaphores[g_CurrentFrameIndex].RetrieveSemaphore() };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			/*
				The next two parameters specify which command buffers to actually submit for execution. As mentioned earlier, we should submit the command buffer that binds the swapchain 
				image we just acquired as color attachment.
			*/
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];

			//The signal semaphore count and pSignalSemaphores parameters specify which semaphores to signal once the command buffer(s) have finished execution. In our case, 
			//we're using the renderFinishedSemaphore for that purpose.
			VkSemaphore signalSemaphores[] = { *m_RenderFinishedSemaphores[g_CurrentFrameIndex].RetrieveSemaphore() };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			//Because we now have more calls to vkWaitForFences, the vkResetFences call should be moved here. Its best to call it right before actually using the fence.
			vkResetFences(*m_Devices->RetrieveLogicalDevice(), 1, &m_InFlightFences[g_CurrentFrameIndex].m_Fence);

			/*
				We can now submit the command buffer to the graphics queue using vkQueueSubmit. The function takes an array of VkSubmitInfo structures as argument for efficiency when 
				the workload is much larger. The last parameter references an optional fence that will be signalled when the command buffers finish execution. We will pass our fence in to signal when a 
				frame has finished.
			*/
			if (vkQueueSubmit(*m_Devices->RetrieveGraphicsQueue(), 1, &submitInfo, m_InFlightFences[g_CurrentFrameIndex].m_Fence) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to submit draw command buffers.");
			}

			/*
				The last step fod drawing a frame is resubmitting the result back to the swapchain to have it actually show up on screen. Presentation is configured through a VkPresentInfoKHR 
				structure at the end of the DrawFrame function.
			*/
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			//The first two parameters specify which semaphores to wait on before presentation can happen, just like VkSubmitInfo. We thus just await for the signal before presentation.
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			//The next parameters specify the swapchains to present images to and the index of the image for each swapchain. This will almost always be a single one.
			VkSwapchainKHR swapchains[] = { m_Swapchain->m_Swapchain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapchains;
			presentInfo.pImageIndices = &imageIndex;

			/*
				There is one last optional parameter called pResults. It allows you to specify an array of VkResult values to check for every individual swapchain to see if presentation 
				was successful. Its not necessary if you're only using a single swapchain, because you can simply use the return value of the present function.
			*/
			presentInfo.pResults = nullptr; //Optional
			result = vkQueuePresentKHR(*m_Devices->RetrievePresentationQueue(), &presentInfo);

			/*
				It is important to do this after vkQueuePresentKHR to ensure that the semaphores are in a consistent state, otherwise a signalled semaphore may never be properly 
				waited upon. Now, to actually detect resizes, we can use the glfwSetFramebufferSizeCallback function in the GLFW framework to setup a callback.
			*/
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
			{
				std::cout << "================================================= Swapchain out of date. Recreating... =================================================\n";
				m_FramebufferResized = false;
				RecreateSwapchain();
			}
			else if (result != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to present swapchain image.\n");
			}

			/*
				If you run your application with validation layers enabled, you may get either errors or notice that the memory usage usually slowly grows. The reason for this is 
				that the application is rapidly submitting work in the DrawFrames function, but doesn't actually check if any of it finishes. If the CPU is submitting work faster 
				than the GPU can keep up, then the queue will slowly fill up with work. Worse, even, is that we are reusing the m_ImageAvaliableSemaphores and m_RenderFinishedSemaphores 
				along with command buffers for multiple frames at the same time. The easy way to solve this is to wait for the work to finish right after submitting it, for example by 
				using vkQueueWaitIdle.

				However, we are likely not optimally using the GPU in this way, because the whole graphics pipeline is only used for one frame at a time right now. The stages that the current 
				frame has already progressed through are idle and could already be used for a next frame. We will now extend our application to allow for multiple frames to be in-flight while 
				still bounding the amount of work that piles up. Start by adding a constant at the top of the program that specifies how many frames should be processed concurrently.

				Although we've now setup the required objects to faciliate processing of multiple frames simulateneously, we still don't actually prevent more than g_MaxFramesInFlight from being 
				submitted. Right now, there is only GPU-GPU synchronization and no CPU-GPU synchronization going on to keep track of how work is going. We may be using the frame #0 objects while
				frame #0 is still in-flight. 

				The perform CPU-GPU synchronization, Vulkan offers a second type of synchronization primitive called fences. Fences are similar to semaphores in the sense that they can be signalled 
				and waited for, but this time, we actually wait for them in our own code. We will first create a fence for each frame. Then, we will adjust our present function to signal the fence 
				that rendering is complete, which the fence is waiting for at the start of this function. This allows for operations to complete before a new frame is drawn.

				The memory leak problem may be gone now, but the program is still not working quite correctly yet. If g_MaxFramesInFlight is higher than the number of swapchain 
				images or vkAcquireNextImageKHR returns images out of order, then its possible that we may start rendering to a swapchain image that is already in flight. To avoid this,
				we need to track for each swapchain image if a frame in flight is currently using it. This mapping will refer to frames in flight by their fences, so we will immediately have
				a synchronization object to wait on before a new frame can use that image.

				We have now implemented all the needed synchronization to ensure that there are no more than 2 frames of work enqueued and that these frames are not accidentally using the same image.
			*/

			vkQueueWaitIdle(*m_Devices->RetrievePresentationQueue());

			//By using the modulo operator, we enmsure that the frame index loop[s around efery g_MaxFramesInFlight enqueued frames.
			g_CurrentFrameIndex = (g_CurrentFrameIndex + 1) % g_MaxFramesInFlight;
		} 

		/*
			Remember that all of the operations we have while drawing frames are asynchronous. That means that when we exit the loop in the main loop, drawing and presentation
			operations may still be going on. Cleaning up resources while that is happening is a bad idea. To fix that, we should wait for the logical device to finish operations
			before exiting the update function and destroying the window. Alternatively, you can also wait for operations in a specific command queue to be finished with vkQueueWaitIdle.
			These functions can be used as a very rudimentary way to perform synchronization. You will see that the program will exit without problems while doing so. 
		*/
		vkDeviceWaitIdle(*m_Devices->RetrieveLogicalDevice());

		CleanupVulkanAssets();
	}

	void VulkanRenderer::CleanupSwapchain()
	{
		m_Swapchain->m_MultisampledColorBufferTexture->DestroyAllTextureInstances();
		m_Swapchain->m_DepthTexture->DestroyAllTextureInstances();

		for (size_t i = 0; i < m_Swapchain->m_SwapchainTextures.size(); i++)
		{
			vkDestroyFramebuffer(*m_Devices->RetrieveLogicalDevice(), m_Swapchain->m_SwapchainFramebuffers[i], nullptr);
		}

		/*
			We could recreate the command pool from scratch, but that is rather wasteful. Instead, we will opt to clean up the existing command buffers with the vkFreeCommandBuffers
			function. This way, we can reuse the existing pool to allocate the new command buffers.
		*/
		vkFreeCommandBuffers(*m_Devices->RetrieveLogicalDevice(), *m_CommandPool->RetrieveCommandPool(), static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());

		m_Pipeline->DestroyPipelineInstance();

		for (int i = 0; i < m_Swapchain->m_SwapchainTextures.size(); i++)
		{
			m_Swapchain->m_SwapchainTextures[i]->DeleteTextureSamplerInstance();
			m_Swapchain->m_SwapchainTextures[i]->DeleteTextureViewInstance();
		}

		for (size_t i = 0; i < m_Swapchain->m_SwapchainTextures.size(); i++)
		{
			vkDestroyBuffer(*m_Devices->RetrieveLogicalDevice(), m_Swapchain->m_UniformBuffers[i], nullptr);
			vkFreeMemory(*m_Devices->RetrieveLogicalDevice(), m_Swapchain->m_UniformBuffersMemory[i], nullptr); //As our uniform buffers depend on the number of swapchain images which could change after recreation, we will clean it up here.
		}

		m_DescriptorPool->DeleteDescriptorPoolInstance();

		vkDestroySwapchainKHR(*m_Devices->RetrieveLogicalDevice(), m_Swapchain->m_Swapchain, nullptr); //Images are created by the swapchain and will be automatically cleaned up as well when it is destroyed. 
		m_Swapchain->m_SwapchainTextures.clear();
	}

	void VulkanRenderer::CleanupVulkanAssets()
	{
		//This is where we destroy the Vulkan instance. For now, rather then leaving them to the destructors, we will do them manually.
		CleanupSwapchain();

		m_ModelTexture->DestroyAllTextureInstances();
		m_DescriptorLayout->DestroyDescriptorLayoutInstance();
		m_IndexBuffer->DestroyIndexBufferInstance();
		m_VertexBuffer->DestroyVertexBufferInstance();

		for (size_t i = 0; i < g_MaxFramesInFlight; i++)
		{
			vkDestroySemaphore(*m_Devices->RetrieveLogicalDevice(), *m_RenderFinishedSemaphores[i].RetrieveSemaphore(), nullptr);
			vkDestroySemaphore(*m_Devices->RetrieveLogicalDevice(), *m_ImageAvaliableSemaphores[i].RetrieveSemaphore(), nullptr);
			vkDestroyFence(*m_Devices->RetrieveLogicalDevice(), m_InFlightFences[i].m_Fence, nullptr);
		}

		m_CommandPool->DestroyCommandPoolInstance();
		m_Devices->DestroyDeviceInstances();
		m_DebugMessenger->DestroyDebugInstance();

		vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
		vkDestroyInstance(m_VulkanInstance, nullptr);

		glfwDestroyWindow(m_Window->RetrieveWindow());
		glfwTerminate();

		std::cout << "Application Terminated.\n" << std::endl;
	}

	void VulkanRenderer::RecreateSwapchain()
	{
		/*
			Minimization will result in a framebuffer size of 0. We will handle that by puasing the window until the window itself is in the foreground again. The initial call
			to glfwGetFramebufferSize handles the size where the size is already correct and glfwWaitEvents will thus have nothing to wait on.
		*/
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_Window->RetrieveWindow(), &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_Window->RetrieveWindow(), &width, &height);
			glfwWaitEvents(); //Await events by putting the current thread to sleep. Once an event is handled, the function returns instantly. In our case, when the width or height isn't 0.
		}

		/*
			Even though the application has successfully rendered your stuff, there are still some circumstances that we will have to handle. It is possible, for example, for the
			window surface to change such that the swapchain is no longer compatible with it. One of the reasons that could cause this to happen is the size of the window changing.
			We have to catch these events and recreate the swapchain as a result. We will call for the CreateSwapchain function along with all of its dependencies here.

			We first call vkDeviceWaitIdle as we usually do as we shouldn't touch resources which may still be in use. Obviously, the first thing we will have to do is recreate the
			swapchain itself. The image views need to be recreated as they are based directly on the swapchain images. The render pass needs to be recreated because it depends on
			the format of the swapchain images. It is rare of the swapchain image format to change during an operation like a window resize, but it should still be handled.
			Viewport and scissor rectangle size is specified during graphics pipeline creation, so the pipeline needs to be rebuilt. It is possible to avoid this by using dynamic state
			for the viewports and scissor rectangles. Finally, the framebuffers and command buffers also directly depend on the swapchain images.

			To make sure that the old versions of these objects are cleaned up before recreating them, we should move some of the cleanup code to a seperate function that we can call
			from here. Note that while calling SelectSwapExtent within CreateSwapchain(), we already query the new window resolution to make sure that the swapchain images have the (new)
			right size, so there's no need to modify ChooseSwapExtent (remember that we already had to use glfwGetFramebufferSize to get the resolution of the surface in pixels when creating
			the swapchain.

			That's all it takes to recreate the swapchain. However, the disadvantage of this approach is that we need to stop all rendering before creating the new swapchain. It is
			possible to create a new swapchain while drawing commands on an image from the old swapchain are still in-flight. You will need to pass the previous swapchain to the
			oldSwapchain field in the VkSwapchainCreateInfoKHR struct and destroy the old swapchain as soon as you've finished using it.
		*/

		vkDeviceWaitIdle(*m_Devices->RetrieveLogicalDevice());
		CleanupSwapchain();

		m_Swapchain->CreateSwapchain();
		//m_Swapchain->RecreateSwapchainImageViews();
		m_Pipeline->CreateRenderPass();
		m_Pipeline->CreateGraphicsPipeline();
		m_Swapchain->CreateMultisampledColorBufferResources(*m_Devices->RetrieveMSAASampleCount());
		m_Swapchain->CreateDepthBufferResources(m_CommandPool->RetrieveCommandPool(), m_Devices->RetrieveGraphicsQueue(), *m_Devices->RetrieveMSAASampleCount());
		m_Swapchain->CreateFramebuffers(m_Pipeline->RetrieveRenderPass());
		m_Swapchain->CreateUniformBuffers();
		m_DescriptorPool->CreateDescriptorPool();
		m_Swapchain->CreateDescriptorSets(m_ModelTexture, m_DescriptorLayout->RetrieveDescriptorSetLayout(), m_DescriptorPool->RetrieveDescriptorPool());
		CreateCommandBuffers();
	}
}	