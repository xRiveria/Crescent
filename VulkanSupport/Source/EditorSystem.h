#pragma once
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../Vendor/imgui/imgui.h"
#include "../Vendor/imgui/imgui_impl_glfw.h"
#include "../Vendor/imgui/imgui_impl_vulkan.h"
#include "Vulkan/VulkanSwapchain.h"
#include <vector>

namespace Crescent
{
	class EditorSystem
	{
	public:
		EditorSystem(GLFWwindow* window, VkInstance* instance, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkQueue* graphicsQueue, VkSurfaceKHR* presentationSurface, VkDescriptorPool* descriptorPool, std::shared_ptr<VulkanSwapchain> swapchain, VkFormat swapchainImageFormat);
		void ShutdownEditor();
		void CleanupEditorResources();
		void RecreateEditorResources();

		void RecordEditorCommands(uint32_t bufferIndex);
		void OnUpdate(std::shared_ptr<VulkanTexture> texture);
		void BeginEditorRenderLoop();
		void EndEditorRenderLoop();

	private:
		void CreateEditorDescriptorPool();
		void CreateEditorCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags);
		void CreateEditorCommandBuffers();
		void CreateEditorRenderPass();
		void CreateEditorFramebuffers();

		void SetEditorDarkThemeColors();
		void RenderDockingContext();

	public:
		std::vector<VkCommandBuffer> m_EditorCommandBuffers;

	private:
		GLFWwindow* m_Window;
		VkInstance* m_VulkanInstance;
		VkPhysicalDevice* m_PhysicalDevice;
		VkDevice* m_LogicalDevice;
		VkQueue* m_GraphicsQueue;
		VkSurfaceKHR* m_PresentationSurface;
		VkDescriptorPool* m_DescriptorPool;
		std::shared_ptr<VulkanSwapchain> m_Swapchain;

		//Editor Specific Resources
		VkFormat m_SwapchainImageFormat;
		std::vector<VkFramebuffer> m_EditorFramebuffers;
		VkCommandPool m_EditorCommandPool;
		VkRenderPass m_EditorRenderPass;
		VkDescriptorPool m_EditorDescriptorPool;
	};
}