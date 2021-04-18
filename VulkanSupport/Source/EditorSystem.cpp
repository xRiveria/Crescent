#include "EditorSystem.h"
#include <vulkan/vulkan.h>
#include "Vulkan/VulkanUtilities.h"

/*
    To Do: Abstract current Vulkan resource creations to construct Editor resources without explicit doings here.
*/

namespace Crescent
{
	static int g_MinimumImageCount = 3;

	EditorSystem::EditorSystem(GLFWwindow* window, VkInstance* instance, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkQueue* graphicsQueue, VkSurfaceKHR* presentationSurface, VkDescriptorPool* descriptorPool, std::shared_ptr<VulkanSwapchain> swapchain, VkFormat swapchainImageFormat) : m_Window(window), m_VulkanInstance(instance), m_PhysicalDevice(physicalDevice), m_LogicalDevice(logicalDevice), 
		m_GraphicsQueue(graphicsQueue), m_PresentationSurface(presentationSurface), m_DescriptorPool(descriptorPool), m_Swapchain(swapchain), m_SwapchainImageFormat(swapchainImageFormat)
	{
		//Setup ImGui Context.
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.Fonts->AddFontFromFileTTF("Resources/Fonts/opensans/OpenSans-Bold.ttf", 17.0f);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("Resources/Fonts/opensans/OpenSans-Regular.ttf", 17.0f);
		(void)io;

		//Setup Dear ImGui Style.
		ImGui::StyleColorsDark();

        //When viewports are enabled, we tweak WindowRounding/WindowBg so Window platforms can look identifical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;

        //SetEditorDarkThemeColors();
        io.FontDefault = io.Fonts->Fonts.back();
        
        //Initialize some ImGui Specific Resources
        CreateEditorDescriptorPool();
        CreateEditorRenderPass();
        /*
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT allows any command buffer allocated from a pool to be individually reset to the initial state, either by calling 
            vkResetCommandBuffer, or via the implicit when calling vkBeginCommandBuffer
        */
        CreateEditorCommandPool(&m_EditorCommandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        CreateEditorCommandBuffers();
        CreateEditorFramebuffers();

		//Setup Platform/Renderer Bindings
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo initializationInfo{};
		initializationInfo.Instance = *m_VulkanInstance;
		initializationInfo.PhysicalDevice = *m_PhysicalDevice;
		initializationInfo.Device = *m_LogicalDevice;

		QueueFamilyIndices queueFamilies = QueryQueueFamilySupport(*m_PhysicalDevice, *m_PresentationSurface);
		initializationInfo.QueueFamily = queueFamilies.m_GraphicsFamily.value();

		initializationInfo.Queue = *m_GraphicsQueue;
		initializationInfo.PipelineCache = VK_NULL_HANDLE; //Empty.
        initializationInfo.DescriptorPool = m_EditorDescriptorPool;
		initializationInfo.Allocator = VK_NULL_HANDLE; //Empty.

		initializationInfo.MinImageCount = g_MinimumImageCount;
		initializationInfo.ImageCount = g_MinimumImageCount;
		initializationInfo.CheckVkResultFn = VK_NULL_HANDLE;

		ImGui_ImplVulkan_Init(&initializationInfo, m_EditorRenderPass);

		 //Upload Fonts
		 {
		 	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(*m_LogicalDevice, m_EditorCommandPool);
			if (ImGui_ImplVulkan_CreateFontsTexture(commandBuffer) != true)
			{
				throw std::runtime_error("Failed to create Fonts for ImGui.");
			}
			else
			{
				std::cout << "Successfully created Fonts for ImGui.\n";
			}
		 	EndSingleTimeCommands(commandBuffer, m_EditorCommandPool, *m_LogicalDevice, *m_GraphicsQueue);
		 	ImGui_ImplVulkan_DestroyFontUploadObjects();
		 }
	}

	void EditorSystem::ShutdownEditor()
	{
        CleanupEditorResources();
        vkDestroyCommandPool(*m_LogicalDevice, m_EditorCommandPool, nullptr);
        vkDestroyRenderPass(*m_LogicalDevice, m_EditorRenderPass, nullptr);
        vkDestroyDescriptorPool(*m_LogicalDevice, m_EditorDescriptorPool, nullptr);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

    void EditorSystem::CleanupEditorResources()
    {
        for (int i = 0; i < m_EditorFramebuffers.size(); i++)
        {
            vkDestroyFramebuffer(*m_LogicalDevice, m_EditorFramebuffers[i], nullptr);
        }

        vkFreeCommandBuffers(*m_LogicalDevice, m_EditorCommandPool, static_cast<uint32_t>(m_EditorCommandBuffers.size()), m_EditorCommandBuffers.data());
    }

    void EditorSystem::RecreateEditorResources()
    {
        CreateEditorCommandBuffers();
        CreateEditorFramebuffers();
    }

    void EditorSystem::OnUpdate(std::shared_ptr<VulkanTexture> texture)
    {
        BeginEditorRenderLoop();
        //RenderDockingContext();

        //1) Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow().
        bool showDemoWindow = true;
        ImGui::ShowDemoWindow(&showDemoWindow);

        //2) Shows a simple window that we create ourselves.
        ImGui::Begin("Diagnostics");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //ImGui::Image(ImGui_ImplVulkan_AddTexture({ *texture->RetrieveTextureSampler(), *texture->RetrieveTextureView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), ImVec2(300, 300));
        ImGui::End();
        //ImGui::End(); //Docking.

        EndEditorRenderLoop();
    }

    //Allocate command buffers equivalent to the size of the amount of textures in our swapchain. 
    void EditorSystem::CreateEditorCommandBuffers() 
    {
        m_EditorCommandBuffers.resize(m_Swapchain->m_SwapchainTextures.size());

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = m_EditorCommandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_EditorCommandBuffers.size());

        if (vkAllocateCommandBuffers(*m_LogicalDevice, &commandBufferAllocateInfo, m_EditorCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Unable to allocate UI command buffers!");
        }
    }

    //Record our ImGui Draw Data into the allocated Command Buffers.
    void EditorSystem::RecordEditorCommands(uint32_t bufferIndex)
    {
        VkCommandBufferBeginInfo bufferBeginInfo = {};
        bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //Specifies that each recording of the command buffer will only be submitted once, and the buffer is then reset and rerecorded again between each submission.
        bufferBeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 

        if (vkBeginCommandBuffer(m_EditorCommandBuffers[bufferIndex], &bufferBeginInfo) != VK_SUCCESS) //Begins recording state.
        {
            throw std::runtime_error("Unable to begin recording of Editor Command Buffer.\n");
        }

        VkClearValue clearColor = { 1.0f, 1.0f, 1.0f, 1.0f };

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_EditorRenderPass;
        renderPassBeginInfo.framebuffer = m_EditorFramebuffers[bufferIndex];
        renderPassBeginInfo.renderArea.extent = m_Swapchain->m_SwapchainExtent;
        //No need for depth here as this is simply an overlay UI.
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        //VK_SUBPASS_CONTENTS_INLINE pecifies that the contents of the subpass will be recorded inline in the primary command buffer, and secondary command buffers must not be executed within the subpass.
        vkCmdBeginRenderPass(m_EditorCommandBuffers[bufferIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Grab and record the draw data from ImGui.
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_EditorCommandBuffers[bufferIndex]);

        // End and submit render pass
        vkCmdEndRenderPass(m_EditorCommandBuffers[bufferIndex]);

        if (vkEndCommandBuffer(m_EditorCommandBuffers[bufferIndex]) != VK_SUCCESS) //Ends recording state.
        {
            throw std::runtime_error("Failed to record Editor Command Buffer.\n");
        }
    }

    void EditorSystem::CreateEditorCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags) 
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

        /*
            Remember that command buffers are executed by submitting them on one of the device queues, like the graphics or presentation queues. Each command pool can only 
            allocate command buffers that are submitted on a single type of queue. As we are currently recording commands for drawing, hence we are going with the graphics
            queue family.
        */
        QueueFamilyIndices queueFamilies = QueryQueueFamilySupport(*m_PhysicalDevice, *m_PresentationSurface);
        commandPoolCreateInfo.queueFamilyIndex = queueFamilies.m_GraphicsFamily.value();

        commandPoolCreateInfo.flags = flags;

        if (vkCreateCommandPool(*m_LogicalDevice, &commandPoolCreateInfo, nullptr, &m_EditorCommandPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("Could not create Editor Command Pool.");
        }
    }

    void EditorSystem::CreateEditorDescriptorPool() 
    {
        VkDescriptorPoolSize poolSizes[] = 
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        /*
            VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT specifies that descriptor sets can return their individual allocations to the pool, i.e. all of vkAllocateDescriptorSets, 
            vkFreeDescriptorSets and vkResetDescriptorPool are allowed. Otherwise, descriptor sets allocated from the pool must not be individually freed back to the pool, i.e. only
            vkAllocateDescriptorSets and vkResetDescriptorPool are allowed.
        */
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 5000 * IM_ARRAYSIZE(poolSizes);
        pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes));
        pool_info.pPoolSizes = poolSizes;

        if (vkCreateDescriptorPool(*m_LogicalDevice, &pool_info, nullptr, &m_EditorDescriptorPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create Editor Descriptor Pool.\n");
        }
    }

    void EditorSystem::CreateEditorFramebuffers() 
    {
        m_EditorFramebuffers.resize(m_Swapchain->m_SwapchainTextures.size());

        VkImageView attachment[1]; //Dummy Array.

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_EditorRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachment;
        framebufferInfo.width = m_Swapchain->m_SwapchainExtent.width;
        framebufferInfo.height = m_Swapchain->m_SwapchainExtent.height;
        framebufferInfo.layers = 1;

        for (uint32_t i = 0; i < m_Swapchain->m_SwapchainTextures.size(); ++i) 
        {
            attachment[0] = *m_Swapchain->m_SwapchainTextures[i]->RetrieveTextureView();
            if (vkCreateFramebuffer(*m_LogicalDevice, &framebufferInfo, nullptr, &m_EditorFramebuffers[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("Failed to create Editor Framebuffer.\n");
            }
        }
    }

    void EditorSystem::CreateEditorRenderPass() 
    {
        // Create an attachment description for the render pass.
        VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.format = m_SwapchainImageFormat;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; //We need our Editor to be drawn on top of our main content. Thus, we preserve the existing data in the attachment.
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //We will transition our contents to a presentation layout when the render pass completes.
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //Specifies that the rendered contents will be stored in memory to be read later.
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        //Create a color attachment reference. Every subpass references one or more of the attachments that we've created.
        VkAttachmentReference attachmentReference = {};
        attachmentReference.attachment = 0;
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Create a subpass
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentReference;

        /*
            Here, we create a subpass dependency to synchronize our main and UI render passes. We want to render the UI after the geometry has been written to the framebuffer.
            Thus, we need to configure a subpass dependency as such.
        */
        VkSubpassDependency subpassDependency = {};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; //Refers to the subpass that happens before the render pass.
        subpassDependency.dstSubpass = 0; //Refers to our subpass here.

        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //Specifies the operations to wait on and the stages in which these operations occur. 
        subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; //We are waiting for writing to be completed in the color attachment before transition.
       
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //The operations that should wait on this to be completed are in the color attachment stage.
        subpassDependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; //It involves the writing of the color attachment. This will prevent the transition from happening until its necessary and allowed.

        // Finally, we create our editor render pass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDescription;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &subpassDependency;

        if (vkCreateRenderPass(*m_LogicalDevice, &renderPassInfo, nullptr, &m_EditorRenderPass) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create Editor Render Pass.\n");
        }
    }

    void EditorSystem::SetEditorDarkThemeColors()
    {
        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    }

    void EditorSystem::BeginEditorRenderLoop()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EditorSystem::EndEditorRenderLoop()
    {
        ImGui::Render();     
        //ImGuiIO& io = ImGui::GetIO();
        //io.DisplaySize = ImVec2(m_Swapchain->m_SwapchainExtent.width, m_Swapchain->m_SwapchainExtent.height); //For default viewport.

        /*
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow* backupCurrentContext = m_Window;
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backupCurrentContext);
            }
        */
    }
    
    void EditorSystem::RenderDockingContext()
    {
        static bool dockSpaceOpen = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->GetWorkPos());
            ImGui::SetNextWindowSize(viewport->GetWorkSize());
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();

        float minimumWindowsize = style.WindowMinSize.x;
        style.WindowMinSize.x = 250.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        style.WindowMinSize.x = minimumWindowsize;
    }
}