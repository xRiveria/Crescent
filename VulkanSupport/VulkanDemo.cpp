#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>


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
		CreateVulkanInstance();
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
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void CreateVulkanInstance()
	{
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

		//As Vulkan is a platform agnostic API, extensions are needed to interface with the window system. GLFW has a handy function that returns the extension(s) it needs to do that.
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); //Returns an array of Vulkan instance extensions and stores the count in a provided buffer.

		creationInfo.enabledExtensionCount = glfwExtensionCount;
		creationInfo.ppEnabledExtensionNames = glfwExtensions;

		//Determines the global validation layers to enable.
		creationInfo.enabledLayerCount = 0;

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
			std::cout << "Successfully created Vulkan Instance.";
		}
	}

private:
	GLFWwindow* m_Window;
	VkInstance m_VulkanInstance;
	const uint32_t m_WindowWidth = 1280;
	const uint32_t m_WindowHeight = 720;
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