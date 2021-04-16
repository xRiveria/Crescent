#include "VulkanDevice.h"
#include <stdexcept>
#include <set>
#include "VulkanUtilities.h"

namespace Crescent
{
	VulkanDevice::VulkanDevice(VkInstance* vulkanInstance, VkSurfaceKHR* vulkanSurface) : m_VulkanInstance(vulkanInstance), m_Surface(vulkanSurface)
	{
		SelectPhysicalDevice(GatherPhysicalDevices());
		CreateLogicalDevice();
	}

	void VulkanDevice::DestroyDeviceInstances()
	{
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	std::vector<VkPhysicalDevice> VulkanDevice::GatherPhysicalDevices()
	{
		//After initializing the library, we need to look for and select a graphics card in the system that supports the features we need.
		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(*m_VulkanInstance, &deviceCount, nullptr);
		if (deviceCount == 0) //No point going further if there are no physical devices with Vulkan support.
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan Support.");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(*m_VulkanInstance, &deviceCount, devices.data());

		for (const VkPhysicalDevice& physicalDevice : devices)
		{
			VkPhysicalDeviceProperties deviceProperties; //Allows us to query basic device properties such as name, type and supported Vulkan versions.
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

			m_GraphicCards.push_back({ deviceProperties.deviceName, deviceProperties.driverVersion, deviceProperties.vendorID, deviceProperties.apiVersion });
		}

		return devices;
	}

	bool VulkanDevice::QueryPhysicalDeviceExtensionsSupport(const VkPhysicalDevice& physicalDevice)
	{
		/*
			Not all graphic cards are capable of presenting images directly to a screen for various reasons, for example because they are designed for servers and don't have
			any display outputs. Secondly, since image presentation is heavily tied to the window system and surfaces associated with Windows, it is not actually part of the
			Vulkan core. We have to enable VK_KHR_swapchain device extension after querying for its support.
		*/
		uint32_t deviceExtensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr); //pLayerName refers to the name of a layer whereby device extensions provided by that layer are returned.
		std::vector<VkExtensionProperties> avaliableExtensions(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, avaliableExtensions.data());

		//Sets are associative containers in which each element has to be unique because the value of the element itself identifies it.
		std::set<std::string> requiredExtensions(m_RequiredDeviceExtensions.begin(), m_RequiredDeviceExtensions.end()); //Constructs a container with as many elements as the range.
		for (const VkExtensionProperties& extension : avaliableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		if (requiredExtensions.empty()) //If empty, all extensions are supported. Else, return false indicating an extension is not supported.
		{
			std::cout << "All required device extensions supported.\n";
			return true;
		}
		else
		{
			return false;
		}
	}

	bool VulkanDevice::QueryPhysicalDeviceSuitability(const VkPhysicalDevice& physicalDevice)
	{
		/*
			Our grading system to select a physical device that best suits our needs, such as ones that support certain functionality including geometry shaders. We implement
			a fun score system to select the best graphics card, adding score increasingly with each supported feature. Naturally, we may even let users select this themselves.
		*/

		//Allows us to query for optional features like texture compression, 64-bit floats and multi viewport rendering.
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

		//===================
		QueueFamilyIndices queueFamilies = QueryQueueFamilySupport(physicalDevice, *m_Surface);
		bool extensionsSupported = QueryPhysicalDeviceExtensionsSupport(physicalDevice);

		//Swapchain support is sufficient for our needs right now if at least one supported image format and one supported presentation mode is avaliable.
		bool swapchainAdequete = false;
		if (extensionsSupported) //It is important that we only try to query for swapchain support after verifying that the swapchain extension is avaliable.
		{
			SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(physicalDevice, *m_Surface);
			swapchainAdequete = !swapchainSupport.m_Formats.empty() && !swapchainSupport.m_PresentationModes.empty();
		} 

		//We will accept it if all requirements have been fulfilled.
		return queueFamilies.IsComplete() && extensionsSupported && swapchainAdequete && deviceFeatures.samplerAnisotropy; //Anisotropic filtering increases the image quality of textures that are at oblique viewing angles.
	}

	void VulkanDevice::SelectPhysicalDevice(const std::vector<VkPhysicalDevice> physicalDevices)
	{
		for (const VkPhysicalDevice& device : physicalDevices)
		{
			if (QueryPhysicalDeviceSuitability(device)) //For now, we accept the first one that is suitable.
			{
				m_PhysicalDevice = device;
				m_MSAAMaxSampleCount = GetMaxUsableSampleCount();

				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(device, &deviceProperties);
				std::cout << "Selected Graphic Card: " << deviceProperties.deviceName << ".\n";

				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to obtain a suitable GPU.");
		}
	}

	VkSampleCountFlagBits VulkanDevice::GetMaxUsableSampleCount()
	{
		/*
			Most modern GPUs support at least 8 samples but this number is not guaranteed to be the same everywhere. By default, we will be using only one sample pixel, 
			which is equivalent to no multisampling, in which case the final image will be remain unchanged. The exact maximum number of samples can be extracted from 
			VkPhysicalDeviceProperties associated with our selected physical device. We're using a depth buffer, so we have to take into account the sample count for both color 
			and depth. The highest sample count that is supported by both (&) will be the maximum we can support. 
		*/
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	void VulkanDevice::CreateLogicalDevice()
	{
		QueueFamilyIndices queueFamilies = QueryQueueFamilySupport(m_PhysicalDevice, *m_Surface);

		//Describes the number of queues we want for a single queue family. The queues are created along with the logical device which we must use a handle to interface with.
		std::vector<VkDeviceQueueCreateInfo> queueCreationInfos;
		std::set<uint32_t> uniqueQueueFamilies = { queueFamilies.m_GraphicsFamily.value(), queueFamilies.m_PresentationFamily.value() };

		/*
			The currently avaliable drivers will only allow you to create a small number of queues for each queue family and you don't really need more than one. That is because
			we can create all of the command buffers on multiple threads and then submit them all at once on the main thread with a single low-overhead call.
			Vulkan allows you to assign priorities to queues to influence the scheduling of command buffer execution using float point numbers between 0.0 and 1.0. This is required
			even if there is only a single queue.
		*/
		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreationInfo{};
			queueCreationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreationInfo.queueFamilyIndex = queueFamily;
			queueCreationInfo.queueCount = 1;
			queueCreationInfo.pQueuePriorities = &queuePriority;

			queueCreationInfos.push_back(queueCreationInfo);
		}

		//Specifies the set of device features that we will be using. These are features that we queried for support with vkGetPhysicalDeviceFeatures.
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE; //Enables anisotrophy.
		/*
			There are certain limitations of our current MSAA implementation which may impact the quality of the output image in more detailed scenes. For example, 
			we're not currently not solving potential problems caused by shader aliasing as MSAA only smoothens out the edges of geometry but not the interior filling. 
			This may lead to a situation when you get a smooth polygon rendered on screen but the applied texture will look aliased if it contains high contrasting colors. 
			One way to approach this problem is to enable sample shading, which will improve quality even further, though at an additional performance cost.
		*/
		deviceFeatures.sampleRateShading = VK_TRUE; //Enables sample shading.

		//Main device creation.
		VkDeviceCreateInfo deviceCreationInfo{};
		deviceCreationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		deviceCreationInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreationInfos.size());
		deviceCreationInfo.pQueueCreateInfos = queueCreationInfos.data();

		deviceCreationInfo.pEnabledFeatures = &deviceFeatures;
		/*
			We now have to specify extensions and validation layers similar to VkInstanceCreateInfo. However, the difference is that these are device specific this time.
			An example of a device specific extension is VK_KHR_swapchain, which allows you to present rendered images from that device to windows. It is possible that there are Vulkan
			devices in the system that lack this ability, for example because they only support compute operations.

			Previous implementations made a distinction between instance and device specific validation layers, but this is no longer the case. This means that enableLayerCount and
			ppEnableLayerNames fields of vkDeviceCreateInfo are ignored by up to date implementations. It is a good idea to set them anyway to be compatible with older implementations.
		*/
		deviceCreationInfo.enabledExtensionCount = static_cast<uint32_t>(m_RequiredDeviceExtensions.size());
		deviceCreationInfo.ppEnabledExtensionNames = m_RequiredDeviceExtensions.data();
		
		/*
			if (m_ValidationLayersEnabled)
			{
				creationInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
				creationInfo.ppEnabledLayerNames = m_ValidationLayers.data();
			}
			else
			{
				creationInfo.enabledLayerCount = 0;
			}
		*/

		if (vkCreateDevice(m_PhysicalDevice, &deviceCreationInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Logical Device.");
		}
		else
		{
			std::cout << "Successfully created Logical Device.\n";
		}

		//We can use vkGetDeviceQueue to retrieve queue handles for each queue family after creation. In case the queue families are the same, the two handles will likely have the same value.
		vkGetDeviceQueue(m_LogicalDevice, queueFamilies.m_GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, queueFamilies.m_PresentationFamily.value(), 0, &m_PresentationQueue);
	}
}