#include "VulkanSwapchain.h"
#include "VulkanUtilities.h"
#include <stdexcept>
#include <iostream>

namespace Crescent
{
	VulkanSwapchain::VulkanSwapchain(VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkSurfaceKHR* presentationSurface, GLFWwindow* window) 
		: m_PhysicalDevice(physicalDevice), m_LogicalDevice(logicalDevice), m_Surface(presentationSurface), m_Window(window)
	{
		CreateSwapchain();
	}

	/*
		We have previously queried for swapchain support. If the conditions were met, then the support is definitely sufficient. However, there may still be many more modes
		with varying optimality. We will now try to determine for the best possible swapchain based on 3 types of settings:

		- Surface Mode (Color Depth)
		- Presentation Mode (Conditions for "swapping" images to the screen).
		- Swap Extent (Resolution of images in the swapchain).
	*/

	VkSurfaceFormatKHR VulkanSwapchain::SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaliableFormats)
	{
		/*
			Each VkSurfaceFormatKHR entry contains a format and a colorSpace member. The format member specifies color channels and types. For example, VK_FORMAT_B8G8R8A8_SRGB
			means that we store the B, R, G and Alpha channels in that order with an 8 bit unsigned integer for a total of 32 bits per pixel. 
			
			The colorSpace member indicates if the SRGB color space is supported or not using the VK_COLOR_SPACE_SRGB_NONLINEAR_KHR flag. For the color space, we will use 
			SRGB if it is avaliable because it results in more accurately perceivec colors. It is pretty much the standard color space for images, like the textures we 
			will use later on. Because of that, we should also use an SRGB color format, a common one being VK_FORMAT_B8G8R8A8_SRGB.
		*/

		for (const VkSurfaceFormatKHR& avaliableFormat : avaliableFormats)
		{
			if (avaliableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && avaliableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return avaliableFormat;
			}
		}

		//If the above fails, we could start ranking the avaliable formats based on how good they are. In most cases, it is okay to settle with the first format specified.

		return avaliableFormats[0];
	}

	VkPresentModeKHR VulkanSwapchain::SelectSwapchainPresentationMode(const std::vector<VkPresentModeKHR>& avaliablePresentationModes)
	{
		/*
			The presentation mode is arguably the most important setting for the swapchain, because it represents the actual conditions for showing images to the screen.
			There are 4 possible modes avalible in Vulkan:

			- VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away, which may result in tearing.

			- VK_PRESENT_MODE_FIFO_KHR: The swapchain is a queue where the display takes an image from the front of the queue when the display is refreshed and the
			program inserts rendered images at the back of the queue. If the queue is full, then the program has to wait. This is most similarly to vertical sync (VSync)
			found in modern games. The moment the display is refrshed is known as "vertical blank".

			- VK_PRESENT_MODE_FIFO_RELAXED_KHR: The mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank.
			Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives. This may result in visible tearing.

			- VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of blocking the application when the queue is full, the images that are
			already queued are simply replaced with the newer ones. This mode can be used to implement triple buffering, which allows you to avoid tearing with its significantly
			less latency issues than standard vertical sync that uses double buffering. 

			VSync is a graphics technology that synchronizes the frame rate of a game and the refresh rate of the monitor. This tech was a way to deal with screen tearing,
			which is when your screen displays portions of multiple frames at one go, resulting in a display that appears split along a line, usually horizontally. Tearing
			occurs when the fresh rate of the monitor (how many times it updates per second) is not in sync with the amount of frames per second.

			Only the VK_PRESENT_MODE_FIFO_KHR mode is guarenteed to be avaliable, so we will again write a function that looks for the best one avaliable.
		*/

		for (const VkPresentModeKHR& presentationMode : avaliablePresentationModes)
		{
			//Triple buffering is a very nice tradeoff as it allows us to prevent tearing whilst still maintaining a fairly low latency by rendering new images that are
			//as up-to-date as possible right until the vertical blank.
			if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return presentationMode;
			}
		}

		//If our preferred optrion isn't avaliable, just return the default one.
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapchain::SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		/*
			The swap extent is the resolution of the swapchain images and its almost always exactly equal to the resolution of the window that we're drawing to in pixels.
			The range of the possible resolutions is defined in the VkSurfaceCapabilitiesKHR structure. Vulkan tells us to match the resolution of the window by setting
			the width and height in the currentExtent member. However, some window managers do allow us to differ here and this is indicated by setting the width
			and height in currentExtent to a special value: the maximum value of uint32_t. In that case, we will pick the resolution that best matches the window without the
			minImageExtent and maxImageExtent bounds. We must specify the resolution in the correct unit.

			GLFW uses 2 units when measuring sizes: pixels and screen coordinates. For example, the resolution (width/height) that we specified earlier when creating the
			window were measured in screen coordinates. However, Vulkan works with pixels, so the swapchain extent must be specified in pixels. Unfortunately,
			if you are using a high DPI display (like Apple's retina display), screen coordinates don't correspond to pixels. Instead, due to the higher pixel density,
			the resolution of the window in pixel will be larger than the resolution in screen coordinates. Thus, if Vulkan doesn't fix the swap extent for us, we can't
			just use the original width and height. Instead, we must use glfwGetFramebufferSize to query the resolution of the window in pixel before matching it against
			the minimum and maximum image extent.
		*/

		if (capabilities.currentExtent.width != UINT32_MAX) //The maximum number that we can store with an unsigned 32-bit integer.
		{
			return capabilities.currentExtent;
		}
		else //We pick the resolution that best matches the window.
		{
			int windowWidth, windowHeight;
			glfwGetFramebufferSize(m_Window, &windowWidth, &windowHeight);
			VkExtent2D actualExtent = { static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight) };

			//The min and max functions are used to clamp the values of width and height between the allowed minimum and maximum extents that are supported by the implementation.
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void VulkanSwapchain::CreateSwapchain()
	{
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(*m_PhysicalDevice, *m_Surface);

		VkSurfaceFormatKHR surfaceFormat = SelectSwapchainSurfaceFormat(swapchainSupport.m_Formats);
		VkPresentModeKHR presentationMode = SelectSwapchainPresentationMode(swapchainSupport.m_PresentationModes);
		VkExtent2D swapExtent = SelectSwapExtent(swapchainSupport.m_Capabilities);

		/*
			Aside from these properties, we also have to decide how many images we would like to have in the swapchain. The implementation specifies the minimum number
			that is requires to function. However, simply sticking to this minimum means thatr we may sometimes have to wait on the driver to complete internal operations
			before we can acquire another image to render to. Therefore, it is recommended to request at least 1 more image than the minimum.
		*/
		uint32_t imageCount = swapchainSupport.m_Capabilities.minImageCount + 1;
		//We should also make sure to not exceed the maximum number of images while doing this, while 0 is a special value that means that there is no maximum.
		if (swapchainSupport.m_Capabilities.maxImageCount > 0 && imageCount > swapchainSupport.m_Capabilities.maxImageCount) //If image support count is more than 0 and our requested count is more than the supported count...
		{
			imageCount = swapchainSupport.m_Capabilities.maxImageCount; //Set to max.
		}

		VkSwapchainCreateInfoKHR swapchainCreationInfo{};
		swapchainCreationInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreationInfo.surface = *m_Surface;
		swapchainCreationInfo.minImageCount = imageCount;
		swapchainCreationInfo.imageFormat = surfaceFormat.format;
		swapchainCreationInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapchainCreationInfo.imageExtent = swapExtent;

		/*
			imageArrayLayers specifies the amount of layers each image consists of. This is always 1 unless you are developing a stereoscopic 3D application. The
			imageUsage bit field specifies what kinds of operations we will be using the images in the swapchain for. Here, we're going to render directly to them,
			which means that they're used as color attachments. It is also possible that you will render images to a seperate image first to perform operations like
			post-processing. In that case, you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a swapchain image.
		*/

		swapchainCreationInfo.imageArrayLayers = 1;
		swapchainCreationInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		/*
			We now specify how to handle swapchain images that will be used across multiple queue families. That will be the case in our application if the graphics
			queue family is different from the presentation queue. We will be drawing on the images in the swapchain from the graphics queue and then submitting them
			on the presentation queue. There are 2 ways to handle images that are accessed from multiple queues:

			- VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explictly transferred before using it in another queue family.
			This offers the best performance.

			- VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership transfers.

			Concurrent mode requires you to specify in advance between which queue families ownership will be shared using queueFamilyIndexCount and pQueueFamilyIndices parameters.
			If the graphics queue family and presentation queue family are the same, which will be the case on most hardware, we should stick to exclusive mode, because
			concurrent mode requires you to specify at least 2 distinct queue families.
		*/
		QueueFamilyIndices queueFamilies = QueryQueueFamilySupport(*m_PhysicalDevice, *m_Surface);
		uint32_t queueFamilyIndices[] = { queueFamilies.m_GraphicsFamily.value(), queueFamilies.m_PresentationFamily.value() };

		if (queueFamilies.m_GraphicsFamily != queueFamilies.m_PresentationFamily) //If the queue families are different, we use Concurrent mode.
		{
			swapchainCreationInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreationInfo.queueFamilyIndexCount = 2;
			swapchainCreationInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			swapchainCreationInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreationInfo.queueFamilyIndexCount = 0; //Optional. Not needed in Exclusive.
			swapchainCreationInfo.pQueueFamilyIndices = nullptr;
		}

		/*
			We can also specify that a certain transform be applied to images in the swapchain if it is supported (supportTransforms in capabilities), like a 90 degree
			closewise rotation or horizontal flip. To specify that you do not need any transformation, simply specify the current transformation.
		*/
		swapchainCreationInfo.preTransform = swapchainSupport.m_Capabilities.currentTransform;

		//The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system. You will almost always want to simply ignore the alpha channel.
		//Hence, use VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
		swapchainCreationInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		/*
			The presentMode member speaks for itself. If the clipped member is set to VK_TRUE, then that means that we don't care about the color of pixels that are obscured,
			for example because another window is in front of them. Unless you really need to be able to read these pixels back and get predictable results, you will get 
			the best peformance by enabling clipping.
		*/
		swapchainCreationInfo.presentMode = presentationMode;
		swapchainCreationInfo.clipped = VK_TRUE;

		/*
			That leaves one last field, oldSwapChain. With Vulkan, it is possible that your swapchain becomes invalid or unoptimized while your application is running, for example
			because the window was resized. In that case, the swapchain actually needs to be recreated from scratch and a reference to the old one must be specified in this field.
			This is rather complex, so for now we will assume that we will only ever create one swapchain.
		*/
		swapchainCreationInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(*m_LogicalDevice, &swapchainCreationInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Swapchain.\n");
		}
		else
		{
			std::cout << "Successfully created Swapchain.\n";
		}

		/*
			Remember thatr we only specified a minimum number of images in the swapchain, so the implementation is allowed to create a swapchain with more.
			Thus, we will first query the final number of images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to retrieve the handles.
		*/
		vkGetSwapchainImagesKHR(*m_LogicalDevice, m_Swapchain, &imageCount, nullptr);
		
		//SwapchainImages stuff.
	}
}