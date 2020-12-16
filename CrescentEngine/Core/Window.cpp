#include "CrescentPCH.h"
#include "Window.h"

namespace CrescentEngine
{
	Window::Window(const std::string& windowName, const float& windowWidth, const float& windowHeight) : m_ApplicationName(windowName), m_WindowWidth(windowWidth), m_WindowHeight(windowHeight)
	{
		CreateWindow(windowName, windowWidth, windowHeight);
	}

	void Window::CreateWindow(const std::string& windowName, const float& windowWidth, const float& windowHeight)
	{
		m_ApplicationName = windowName;
		m_WindowWidth = windowWidth;
		m_WindowHeight = windowHeight;

		InitializeGLFW();
		m_ApplicationWindow = glfwCreateWindow((int)m_WindowWidth, (int)m_WindowHeight, m_ApplicationName.c_str(), nullptr, nullptr);

		if (m_ApplicationWindow == nullptr)
		{
			CrescentError("Failed to create GLFW Window.");
		}
		CrescentInfo("Successfully created GLFW Window.");

		glfwMakeContextCurrent(m_ApplicationWindow);
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void Window::InitializeGLFW()
	{
		if (!glfwInit())
		{
			CrescentError("Failed to initialize GLFW.");
		}
		CrescentInfo("Successfully initialized GLFW.");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
}