#include "Window.h"
#include <iostream>

namespace Crescent
{
	Window::Window(const uint32_t& windowWidth, const uint32_t& windowHeight, const std::string& windowName) : m_WindowWidth(windowWidth), m_WindowHeight(windowHeight), m_WindowName(windowName)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //GLFW was originally designed to create an OpenGL context. As such, we need to tell it to not create an OpenGL context.
		m_Window = glfwCreateWindow(windowWidth, windowHeight, windowName.c_str(), nullptr, nullptr);

		std::cout << "Created Window named " << windowName << " with size " << windowWidth << "x" << windowHeight << ".\n";
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_Window);
	}
}