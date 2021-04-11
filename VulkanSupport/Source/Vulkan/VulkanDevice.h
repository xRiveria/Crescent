#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <iostream>

namespace Crescent
{
	struct GraphicCard
	{
		GraphicCard(const std::string& name, const uint32_t& driverVersion, const uint32_t& vendorID, const uint32_t& apiVersion) : m_Name(name), m_DriverVersion(driverVersion), m_VendorID(vendorID), m_APIVersion(apiVersion)
		{
			std::cout << "Found Graphic Card: " << name << ".\n";
		}

		std::string m_Name;
		uint32_t m_DriverVersion;
		uint32_t m_VendorID;
		uint32_t m_APIVersion;
	};

	class VulkanDevice
	{
	public:
		VulkanDevice(VkInstance* vulkanInstance, VkSurfaceKHR* vulkanSurface);
		void DestroyDeviceInstances();

		VkDevice* RetrieveLogicalDevice() { return &m_LogicalDevice; }
		VkPhysicalDevice* RetrievePhysicalDevice() { return &m_PhysicalDevice; }
		VkQueue* RetrieveGraphicsQueue() { return &m_GraphicsQueue; }
		VkQueue* RetrievePresentationQueue() { return &m_PresentationQueue; }

	private:
		std::vector<VkPhysicalDevice> GatherPhysicalDevices();
		bool QueryPhysicalDeviceExtensionsSupport(const VkPhysicalDevice& physicalDevice);
		bool QueryPhysicalDeviceSuitability(const VkPhysicalDevice& physicalDevice);
		void SelectPhysicalDevice(const std::vector<VkPhysicalDevice> physicalDevices);

		void CreateLogicalDevice();

	private:
		std::vector<GraphicCard> m_GraphicCards;
		const std::vector<const char*> m_RequiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkQueue m_GraphicsQueue;
		VkQueue m_PresentationQueue;
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_LogicalDevice;

		VkSurfaceKHR* m_Surface;
		VkInstance* m_VulkanInstance;
	};
}