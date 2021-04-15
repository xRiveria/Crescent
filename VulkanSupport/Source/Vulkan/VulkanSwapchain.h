#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <memory>
#include "VulkanTexture.h"

namespace Crescent
{
	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkSurfaceKHR* presentationSurface, GLFWwindow* window);
		void DestroySwapchainInstance();

		void CreateSwapchain();
		void CreateDepthBufferResources(VkCommandPool* commandPool, VkQueue* queue);
		void CreateFramebuffers(VkRenderPass* renderPass);
		void CreateUniformBuffers();
		void CreateDescriptorSets(std::shared_ptr<VulkanTexture> texture, VkDescriptorSetLayout* descriptorLayout, VkDescriptorPool* descriptorPool);
		void RecreateSwapchainImageViews();
		
		std::vector<std::shared_ptr<VulkanTexture>>* RetrieveSwapchainImages() { return &m_SwapchainTextures; }
		VkFormat RetrieveSwapchainImageFormat() const { return m_SwapchainFormat; }
		VkExtent2D* RetrieveSwapchainExtent() { return &m_SwapchainExtent; }
		VkSwapchainKHR* RetrieveSwapchain() { return &m_Swapchain; }
		//std::vector<VkDeviceMemory>* RetrieveUniformBufferMemory() { return &m_UniformBuffersMemory; }

	private:
		VkSurfaceFormatKHR SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaliableFormats);
		VkPresentModeKHR SelectSwapchainPresentationMode(const std::vector<VkPresentModeKHR>& avaliablePresentationModes);
		VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	public:
		//Buffers
		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		std::vector<VkDescriptorSet> m_DescriptorSets;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;

	public:
		VkSwapchainKHR m_Swapchain;
		VkFormat m_SwapchainFormat;
		VkExtent2D m_SwapchainExtent;
		std::vector<std::shared_ptr<VulkanTexture>> m_SwapchainTextures;
		std::shared_ptr<VulkanTexture> m_DepthTexture = nullptr;
		
		GLFWwindow* m_Window;
		VkSurfaceKHR* m_Surface;
		VkPhysicalDevice* m_PhysicalDevice;
		VkDevice* m_LogicalDevice;
	};
}