#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace Crescent
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR m_Capabilities; //Minimum or maximum number of images in the swapchain, the minimum/maximum width and height of the images etc.
		std::vector<VkSurfaceFormatKHR> m_Formats; //Pixel Format/Color Space.
		std::vector<VkPresentModeKHR> m_PresentationModes; //Avaliable presentation modes.
	};

	struct QueueFamilyIndices
	{
		/*
			We use optional here to indicate whether a particular queue family was found. This is good as while we may "prefer" devices with a dedicated transfer queue family,
			we may not actually require it. It is likely that the queues themselves end up being from the same queue family, but throughout our program, we will treat them
			as thoughg they were seperate queues for a uniform approach. Nevertheless, we could add logic that explicitly prefers a physical device that supports
			drawing and presentation in the same queue for improved performance.
		*/
		std::optional<uint32_t> m_GraphicsFamily;
		std::optional<uint32_t> m_PresentationFamily; //While Vulkan supports WSI (Windows System Integration), it doesn't mean that every device will support it.

		bool IsComplete()
		{
			return m_GraphicsFamily.has_value() && m_PresentationFamily.has_value();
		}
	};

	static QueueFamilyIndices QueryQueueFamilySupport(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& presentationSurface)
	{
		/*
			Almost every operation in Vulkan from drawing to texture uploads require commands to be submitted to a queue. Thre are different queues that a device may support,
			such as a graphics rendering queue, texture uploading queue etc. In Vulkan, queues are collected into queue families. These queue families each contain
			a sedt of queues which all support the same type of operations. We thus need to check which queue families are supported by the device and which ones
			support the commands that we want to use.
		*/

		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			VkBool32 presentationSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, presentationSurface, &presentationSupport);

			if (presentationSupport) //If any supports presentation queues...
			{
				indices.m_PresentationFamily = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) //If any supports graphics queues...
			{
				indices.m_GraphicsFamily = i;
			}

			if (indices.IsComplete()) //If all conditions were already fulfilled, exit the loop.
			{
				break;
			}

			i++;
		}

		return indices;
	}

	static SwapchainSupportDetails QuerySwapchainSupport(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& presentationSurface)
	{
		/*
			Vulkan does not have the concept of a default framebuffer. Hence, it requires an infrastructure that will own the buffers we will render to before we visualize
			them on the screen. This infrastructure is known as the swapchain and must be created explictly in Vulkan. The swapchain is essentially a queue of images that
			are waiting to be presented to the screen. Our application will acquire such an image to draw to it and then return it to the queue. How exactly the queue
			works and the conditions for presenting an image from the queue depends on how the swapchain is setup, but the general purpose of the swapchain is to synchronize
			the presentation of images with the refresh rate of the screen (how many times the screen is updated with new frames per second).
		*/
		SwapchainSupportDetails swapchainDetails;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, presentationSurface, &swapchainDetails.m_Capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentationSurface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			swapchainDetails.m_Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentationSurface, &formatCount, swapchainDetails.m_Formats.data());
		}

		uint32_t presentationModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentationSurface, &presentationModeCount, nullptr);
		if (presentationModeCount != 0)
		{
			swapchainDetails.m_PresentationModes.resize(presentationModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentationSurface, &presentationModeCount, swapchainDetails.m_PresentationModes.data());
		}

		return swapchainDetails;
	}
}