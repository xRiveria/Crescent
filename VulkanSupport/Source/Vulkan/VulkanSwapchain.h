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

		VkFormat RetrieveSwapchainImageFormat() const { return m_SwapchainFormat; }
		VkExtent2D* RetrieveSwapchainExtent() { return &m_SwapchainExtent; }

	private:
		VkSurfaceFormatKHR SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaliableFormats);
		VkPresentModeKHR SelectSwapchainPresentationMode(const std::vector<VkPresentModeKHR>& avaliablePresentationModes);
		VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void CreateSwapchain();

	private:
		VkSwapchainKHR m_Swapchain;
		VkFormat m_SwapchainFormat;
		VkExtent2D m_SwapchainExtent;
		std::vector<std::shared_ptr<VulkanTexture>> m_SwapchainTextures;
		
		GLFWwindow* m_Window;
		VkSurfaceKHR* m_Surface;
		VkPhysicalDevice* m_PhysicalDevice;
		VkDevice* m_LogicalDevice;
	};
}