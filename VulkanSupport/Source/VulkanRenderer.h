#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <memory>
#include "../Source/Vulkan/VulkanDebug.h"
#include "../Source/Vulkan/VulkanDevice.h"
#include "../Source/Vulkan/VulkanSwapchain.h"
#include "../Source/Vulkan/VulkanDescriptorLayout.h"
#include "../Source/Vulkan/VulkanPipeline.h"
#include "../Source/Vulkan/VulkanCommandPool.h"
#include "../Source/Vulkan/VulkanResource.h"
#include "../Source/Vulkan/VulkanVertexBuffer.h"
#include "../Source/Vulkan/VulkanIndexBuffer.h"
#include "../Source/Vulkan/VulkanDescriptorPool.h"
#include "../Source/Vulkan/VulkanFence.h"
#include "../Source/Vulkan/VulkanSemaphore.h"
#include "Window.h"
#include "EditorSystem.h"

namespace Crescent
{
	class VulkanRenderer
	{
	public:
		VulkanRenderer(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion, const bool& validationLayersEnabled);
		~VulkanRenderer();

		void CleanupSwapchain();
		void CleanupVulkanAssets();
		void DrawFrames();

	private:
		void CreatePresentationSurface();
		void CreateVulkanInstance(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion);
		void UpdateUniformBuffers(uint32_t currentImage);
		void CreateSynchronizationObjects();
		void CreateCommandBuffers();
		void RecreateSwapchain();

	public:
		bool m_FramebufferResized = false;

	private:
		VkInstance m_VulkanInstance;
		std::shared_ptr<VulkanDebug> m_DebugMessenger = nullptr;
		std::shared_ptr<VulkanDevice> m_Devices = nullptr;
		std::shared_ptr<VulkanSwapchain> m_Swapchain = nullptr;
		std::shared_ptr<VulkanDescriptorLayout> m_DescriptorLayout = nullptr;
		std::shared_ptr<VulkanPipeline> m_Pipeline = nullptr;
		std::shared_ptr<VulkanCommandPool> m_CommandPool = nullptr;
		std::shared_ptr<VulkanDescriptorPool> m_DescriptorPool = nullptr;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		//Presentation
		std::shared_ptr<Window> m_Window = nullptr;
		VkSurfaceKHR m_Surface;

		//Validation
		bool m_ValidationLayersEnabled = false;

		//Synchronization
		std::vector<VulkanSemaphore> m_ImageAvaliableSemaphores; //Each frame should have its own set of semaphores.
		std::vector<VulkanSemaphore> m_RenderFinishedSemaphores;
		std::vector<VulkanFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlightFences;

		//Custom - Possible to combine them?
		std::shared_ptr<VulkanTexture> m_ModelTexture = nullptr;
		std::shared_ptr<VulkanResource> m_Model = nullptr;
		std::shared_ptr<VulkanVertexBuffer> m_VertexBuffer = nullptr;
		std::shared_ptr<VulkanIndexBuffer> m_IndexBuffer = nullptr;

		//Editor
		std::shared_ptr<EditorSystem> m_Editor = nullptr;
	};
}
