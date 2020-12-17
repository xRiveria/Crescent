#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

namespace CrescentEngine
{
	class Window
	{
	public:
		Window() {}
		Window(const std::string& windowName, const float& windowWidth, const float& windowHeight);

		void CreateWindow(const std::string& windowName, const float& windowWidth, const float& windowHeight);

		//Callbacks
		void SetFramebufferCallback(GLFWframebuffersizefun callback);
		void SetMouseButtonCallback(GLFWmousebuttonfun callback);
		void SetMouseScrollCallback(GLFWscrollfun callback);
		void SetMouseCursorCallback(GLFWcursorposfun callback);

		GLFWwindow* RetrieveWindow() const { return m_ApplicationWindow; }
		float RetrieveWindowWidth() const { return m_WindowWidth; }
		float RetrieveWindowHeight() const { return m_WindowHeight; }

	private:
		void InitializeGLFW();

	private:
		GLFWwindow* m_ApplicationWindow = nullptr;
		std::string m_ApplicationName = "Application";

		float m_WindowWidth = 1280.0f;
		float m_WindowHeight = 720.0f;
	};
}
