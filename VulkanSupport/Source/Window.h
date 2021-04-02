#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace Crescent
{
	class Window
	{
	public:
		Window(const uint32_t& windowWidth, const uint32_t& windowHeight, const std::string& windowName);
		~Window();

		GLFWwindow* RetrieveWindow() const { return m_Window; }

	private:
		GLFWwindow* m_Window = nullptr;

		std::string m_WindowName = "Window";
		uint32_t m_WindowWidth = 800;
		uint32_t m_WindowHeight = 600;
	};
}