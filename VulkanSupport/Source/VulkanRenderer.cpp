#include "VulkanRenderer.h"
#include <stdexcept>
#include <iostream>
#include <vector>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Crescent
{
	VulkanRenderer::VulkanRenderer(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion, const bool& validationLayersEnabled)
		: m_ValidationLayersEnabled(validationLayersEnabled)
	{
		m_Window = std::make_shared<Window>(1280, 1080, "Vulkan Demo");
		m_DebugMessenger = std::make_shared<VulkanDebug>(); //To be created first before the instance so as to allow for callbacks during instance creation.

		CreateVulkanInstance(applicationName, engineName, applicationMainVersion, applicationSubVersion);
		if (validationLayersEnabled)
		{
			m_DebugMessenger->SetupDebugMessenger(&m_VulkanInstance);
		}
		CreatePresentationSurface();
		m_Devices = std::make_shared<VulkanDevice>(&m_VulkanInstance, &m_Surface);
		m_Swapchain = std::make_shared<VulkanSwapchain>(m_Devices->RetrievePhysicalDevice(), m_Devices->RetrieveLogicalDevice(), &m_Surface, m_Window->RetrieveWindow());
		m_Pipeline = std::make_shared<VulkanPipeline>(m_Swapchain->RetrieveSwapchainImageFormat(), m_Devices->RetrievePhysicalDevice(), m_Devices->RetrieveLogicalDevice());
	}

	VulkanRenderer::~VulkanRenderer()
	{

	}

	//Retrieves the names of extensions required by our Vulkan application.
	std::vector<const char*> RetrieveRequiredVulkanExtensions(const bool& validationLayersEnabled)
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

	void VulkanRenderer::DrawFrames()
	{
		while (!glfwWindowShouldClose(m_Window->RetrieveWindow()))
		{
			glfwPollEvents();
		}

		//This is where we destroy the Vulkan instance. For now, rather then leaving them to the destructors, we will do them manually.
		m_Swapchain->DestroySwapchainInstance();
		m_Devices->DestroyDeviceInstances();
		vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
		m_DebugMessenger->DestroyDebugInstance();
		vkDestroyInstance(m_VulkanInstance, nullptr);
	}
}