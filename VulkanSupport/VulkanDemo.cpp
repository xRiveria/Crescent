#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

//As this is an extension function, it is not automatically loaded. We will thus look up its address ourselves by using vkGetInstanceProcAddr.
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

class VulkanApplication
{
public:
	void Run()
	{
		InitializeWindowLibrary();
		InitializeVulkan();
		MainLoop(); 
		CleanUp(); //Deallocate resources and end the program.
	}

private:
	void InitializeWindowLibrary()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //GLFW was originally designed to create an OpenGL context. We need to tell it to not create an OpenGL context.
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //Handling resized windows also takes special care that we will look into later. Disable it for now.
		m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "Crescent Engine", nullptr, nullptr);
	}

	void InitializeVulkan()
	{
		QueryExtensionsSupport();
		QueryValidationLayersSupport();
		CreateVulkanInstance();
		SetupDebugMessenger();
		PickPhysicalDevice();
		CreateLogicalDevice();
	}

	void MainLoop()
	{
		while (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();
		}
	}

	void CleanUp()
	{
		//Every Vulkan object that we create needs to be explicitly destroyed when we no longer need it.
		//Vulkan's niche is to be explicit about every operation. Thus, its good to be about the lifetime of objects as well.
		vkDestroyDevice(m_Device, nullptr); //Logical devices don't interact directly with instances, which is why it isn't needed as a parameter.

		if (m_ValidationLayersEnabled)
		{
			DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, nullptr);
		}

		vkDestroyInstance(m_VulkanInstance, nullptr); //All Vulkan resources should be destroyed before the instance is destroyed.
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	std::vector<const char*> RetrieveRequiredExtensions()
	{
		//As Vulkan is a platform agnostic API, extensions are needed to interface with the window system. GLFW has a handy function that returns the extension(s) it needs to do that.
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); //Returns an array of Vulkan instance extensions and stores the count in a provided buffer.

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (m_ValidationLayersEnabled)
		{
			//Note that the extensions listed by GLFW in glfwGetRequiredInstanceExtensions are always required, but the debug messenger extension is conditionally added.
			//While the validation layers will print debug messages to the standard output by default, we can choose to handle this ourselves by providing an explicit 
			//callback in our program. This will also allow us to decide what kind of messages we want to see as not all are necessarily (fatal) errors. 
			//VK_EXT_debug_utils allow us to setup a debug messenger to handle debug messages and their associated details. 
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); //VK_EXT_DEBUG_UTILS_EXTENSION_NAME is equal to the literal string VK_EXT_debug_utils.
		}

		return extensions;
	}

	//How a debug callback function looks like. The VKAPI_ATTR and VKAPI_CALL ensures that the function has the right signature for Vulkan to call it.
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		/*
			The first parameter specifies the severity of the message which is one of the following flags:
			- VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic messages.
			- VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the creation of a resource.
			- VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior that may not be an error but very likely a bug in your application.
			- VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes.

			The second parameter specifies a message type filter for the logs you wish to see get printed in addition to the severity level.
			- VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance.
			- VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake.
			- VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan.

			The third parameter refers to a VkDebugUtilsMessengerCallbackDataEXT struct containing the details of the message itself with the most important ones being:
			- pMessage: The debug message as a null terminated string.
			- pObjects - Array of Vulkan object handles related to the message.
			- objectCount - Number of objects in said array.
		*/

		//The values of the enumerations are setup in such a way that we can use a comparison operator to check if a message is equal or worse compared to some level of severity.
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			//Message is important enough to show.
		}

		std::cerr << "Validation Layer: " << pCallbackData->pMessage << "\n";

		//This boolean that we return indicates if the Vulkan call that triggered the validation layer messege should be aborted. If this returns true, then the call is
		//aborted with the VK_ERROR_VALIDATION_FAILED_EXT error. 
		return VK_FALSE;
	}
	
	void QueryExtensionsSupport()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "Avaliable Extensions: \n";
		for (const auto& extension : extensions)
		{
			std::cout << '\t' << extension.extensionName << '\n';
		}
	}

	bool QueryValidationLayersSupport()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> avaliableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, avaliableLayers.data());

		//Check if the validation layers we want are supported.
		for (const char* layerName : m_ValidationLayers)
		{
			bool layerFound = false;
			for (const auto& layerProperties : avaliableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0) //If they are identical, 0 is returned.
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	void CreateVulkanInstance()
	{
		//Validation layers are optional components that hook into Vulkan function calls to apply operations such as parameter checking, Vulkan object tracking, call logging etc.
		if (m_ValidationLayersEnabled && !QueryValidationLayersSupport()) //If validation layers are enabled but our layers are not supported...
		{
			throw::std::runtime_error("Validation Layers Requested, but not avaliable!");
		}

		/*
			A Vulkan instance is used to initialize the Vulkan library. This instance is the connection between your application and the Vulkan library and involves 
			specifying details about your application to the driver. This is done through the Application Info struct. While this data is techically optional, 
			it allows the application to pass information about itself to the implementation. Compared to several APIs out there that use function parameters to pass 
			information, a lot of these information is passed through structs in Vulkan instead.
		*/
		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; //Many structs in Vulkan require you to explicitly specify the type of it in this sType member.
		applicationInfo.pApplicationName = "Application Demo";
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); //Supplied by us, the developers. This is the version of the application.
		applicationInfo.pEngineName = "Crescent Engine";
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); //Supplied by us, the developers. This is the version of the engine.
		applicationInfo.apiVersion = VK_API_VERSION_1_0; //The highest version of Vulkan that the application is designed to use. 

		//The Instance Creation struct tells the Vulkan driver which global extensions and validation layers we want to use. These settings apply to the entire program.
		VkInstanceCreateInfo creationInfo{};
		creationInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		creationInfo.pApplicationInfo = &applicationInfo;

		std::vector<const char*> requiredExtensions = RetrieveRequiredExtensions();
		creationInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		creationInfo.ppEnabledExtensionNames = requiredExtensions.data();
		
		//For custom debugging for our instance creation call. This isn't possible to do usually as our debugger is created after the instance itself is created. This is why
		//we populate the pNext struct member with our debug information so that it will be used automatically during vkCreateInstance and vkDestroyedInstance and cleaned up after that.
		VkDebugUtilsMessengerCreateInfoEXT debugCreationInfo; //This is placed outside the if statement so that it is not destroyed before the vkCreateInstance call below.
		if (m_ValidationLayersEnabled) 
		{
			creationInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size()); 
			creationInfo.ppEnabledLayerNames = m_ValidationLayers.data(); //Determines the global validation layers to enable based on what we specified.

			PopulateDebugMessengerCreationInfo(debugCreationInfo);
			creationInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreationInfo;
		}
		else
		{
			creationInfo.enabledLayerCount = 0;
			creationInfo.pNext = nullptr;
		}

		//Lastly, we can create our instance. Nearly all Vulkan functions return a value of type VkResult that either VK_SUCCESS or an error code. 
		//The general pattern of object creation parameters in Vulkan is as follows:
		//- Pointer to struct with its creation info.
		//- Pointer to custom allocator callbacks.
		//- Pointer to the variable that stores the handle to the new object.
		if (vkCreateInstance(&creationInfo, nullptr, &m_VulkanInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create instance.");
		}
		else
		{
			std::cout << "Successfully created Vulkan Instance." << "\n";
		}
	}

	void PopulateDebugMessengerCreationInfo(VkDebugUtilsMessengerCreateInfoEXT& creationInfo)
	{
		//Naturally, even a debug callback in Vulkan is managed with a handle that needs to be explicitly created and destroyed. Such a callback is part of a debug messenger
		//object and we can have as many of them as we want.
		creationInfo = {};
		creationInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		//This field allows you to specify all the types of severities you would like your callback to be called for. We've specified all types except 
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT and VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT to receive notifications about possible problems while leaving out verbose general debug information.
		creationInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		//Similarly, the messageType field lets you filter which types of messages your callback is notified about. We've simply enabled all types here.
		creationInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		//Specifies a pointer to the callback option. You can pass a pointer here that will be passed along to the callback function via the pUserData parameter.
		creationInfo.pfnUserCallback = DebugCallback;
		creationInfo.pUserData = nullptr; //Optional
	}

	void SetupDebugMessenger()
	{
		if (!m_ValidationLayersEnabled) { return; }

		VkDebugUtilsMessengerCreateInfoEXT creationInfo;
		PopulateDebugMessengerCreationInfo(creationInfo);

		if (CreateDebugUtilsMessengerEXT(m_VulkanInstance, &creationInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to setup debug messenger.");
		}
	}

	struct QueueFamilyIndices
	{
		//We use optional here to indicate whether a particular queue family was found. This is good as while we may *prefer* devices with a dedicated transfer queue family, but not require it.
		std::optional<uint32_t> m_GraphicsFamily;

		bool IsComplete()
		{
			return m_GraphicsFamily.has_value();
		}
	};

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
	{
		//Almost every operation in Vulkan from drawing to texture uploads requires commands to be submitted to a queue. There are different queues that a device may support,
		//such as a graphics rendering queue, texture uploading queue etc. In Vulkan, queues are collected together into queue families. These queue families each contain
		//a set of queues which all support the same type of operations. We thus need to check which queue familes are supported by the device and which one supports the commands
		//that we want to use. 

		//Logic to find a graphics queue family.
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) //If anyone supports graphics queues...
			{
				indices.m_GraphicsFamily = i;
			}

			if (indices.IsComplete()) //If all conditions are already fulfilled, exit the loop.
			{
				break;
			}

			i++;
		}

		return indices;
	}

	bool IsPhysicalDeviceSuitable(VkPhysicalDevice device)
	{
		//Our grading system to select a physical device. We can select graphic cards that support certain functionality that we want, such as geometry shaders,
		//or even create a score system to select the best graphic card, adding score increasingly with each supported feature. Alternatively, we may even let users select them themselves.
		VkPhysicalDeviceProperties deviceProperties; //This allows us to query basic device properties such as the name, type and supported Vulkan versions.
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		std::cout << "Found Physical Device: " << deviceProperties.deviceName;

		//This allows us to query for optional features like texture compression, 64 bit floats and multi viewport rendering.
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		QueueFamilyIndices indices = FindQueueFamilies(device);

		//We will accept it if all requirements were fulfilled.
		return indices.IsComplete();
	}

	void PickPhysicalDevice()
	{
		//After initializing the library, we need to look for and select a graphics card in the system that supports the features we need. In fact, we can select any number
		//of graphics cards and use them simultaneously, but for now, we will stick to the first one that suits our needs.
		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);
		if (deviceCount == 0) //No point going further if there are 0 devices with Vulkan support.
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan Support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, devices.data());

		for (const VkPhysicalDevice& device : devices)
		{
			if (IsPhysicalDeviceSuitable(device)) 
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	void CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

		//Describes the number of queues we want for a single queue family. The queues are created along with the logical device which we must use a handle to interface with.
		VkDeviceQueueCreateInfo queueCreationInfo{};
		queueCreationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreationInfo.queueFamilyIndex = indices.m_GraphicsFamily.value();
		queueCreationInfo.queueCount = 1;

		//The currently avaliable drivers will only allow you to create a small number of queues for each queue family and you don't really need more than one.
		//That's because you can create all of the command buffers on multiple threads and then submit them all at once on the main thread with a single low-overhead call.
		//Vulkan allows you to assign priorities to queues to influence the scheduling of command buffer execution using floating point numbers between 0.0 and 1.0. This is required even if there is only a single queue.
		float queuePriority = 1.0f;
		queueCreationInfo.pQueuePriorities = &queuePriority;

		//Specifies the set of device features that we will be using. These are the features that we queried support for with vkGetPhysicalDeviceFeatures.
		VkPhysicalDeviceFeatures deviceFeatures{};

		//Main device creation info struct.
		VkDeviceCreateInfo creationInfo{};
		creationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		creationInfo.pQueueCreateInfos = &queueCreationInfo;
		creationInfo.queueCreateInfoCount = 1;
		creationInfo.pEnabledFeatures = &deviceFeatures;
		//We now have to specify extensions and validation layers similar to VkInstanceCreateInfo. However, the difference is that these are device specific this time.
		//An example of a device specific extension is VK_KHR_swapchain, which allows you to present rendered images from that device to windows. It is possible that there are
		//Vulkan devices in the system that lack this ability, for example because they only support compute operations.
		//Previous implementations made a distinction between instance and device specific validation layers, but this is no longer the case. This means that enabledLayerCount
		//and ppEnabledLayerNames fields of vkDeviceCreateInfo are ignored by up-to-date implementations. It is a good idea to set them anyway to be compatible with older implementations.
		creationInfo.enabledExtensionCount = 0;

		if (m_ValidationLayersEnabled)
		{
			creationInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			creationInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else
		{
			creationInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_PhysicalDevice, &creationInfo, nullptr, &m_Device) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device.");
		}

		//We can use vkGetDeviceQueue to retrieve queue handles for each queue family.
		vkGetDeviceQueue(m_Device, indices.m_GraphicsFamily.value(), 0, &m_GraphicsQueue);
	}

private:
	GLFWwindow* m_Window;
	const uint32_t m_WindowWidth = 1280;
	const uint32_t m_WindowHeight = 720;

	VkInstance m_VulkanInstance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkPhysicalDevice m_PhysicalDevice; //Implictly destroyed when the VkInstance is destroyed.
	VkDevice m_Device; //Used to interface with the physical device. Multiple logical devices can be created from the same physical device.
	VkQueue m_GraphicsQueue; //Used to interface with the queues in our logical device. This is implicitly destroyed with the device. 

	const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
	const bool m_ValidationLayersEnabled = false;
#else
	const bool m_ValidationLayersEnabled = true;
#endif
};

int main(int argc, int argv[])
{
	VulkanApplication application;

	try
	{
		application.Run();
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << std::endl;
		return EXIT_FAILURE; //Unsuccessful program execution.
	}

	return EXIT_SUCCESS; //Successful program execution.
}