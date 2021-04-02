#include "Application.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

namespace Crescent
{
	Application::Application(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion)
		: m_ApplicationName(applicationName), m_EngineName(engineName), m_ApplicationMainVersion(applicationMainVersion), m_ApplicationSubVersion(applicationSubVersion)
	{
		InitializeGLFWLibrary();
		m_Renderer = std::make_shared<VulkanRenderer>(m_ApplicationName, m_EngineName, m_ApplicationMainVersion, m_ApplicationSubVersion, true);
	}

	Application::~Application()
	{
		glfwTerminate();
	}

	void Application::OnUpdate()
	{
		m_Renderer->DrawFrames();
	}

	void Application::InitializeGLFWLibrary()
	{
		if (!glfwInit())
		{
			throw std::runtime_error("GLFW initialization failed.\n");
		}
		else
		{
			std::cout << "Successfully initialized GLFW.\n";
		}
	}
}