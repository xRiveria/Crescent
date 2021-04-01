#include "Core/Application.h"
#include <iostream>

namespace Crescent
{
	class VulkanDemo : public Application
	{
	public:
		VulkanDemo() : Application("Vulkan Demo", "Crescent", 0, 1)
		{

		}

		~VulkanDemo()
		{

		}
	};

	Crescent::Application* CreateApplication()
	{
		return new VulkanDemo();
	}
}

int main(int argc, int argv[])
{
	Crescent::Application* application = Crescent::CreateApplication();

	application->OnUpdate();

	delete application;
}