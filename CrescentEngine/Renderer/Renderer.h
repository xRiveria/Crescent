#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace CrescentEngine
{
	class Renderer
	{
	public:
		Renderer() {}

		void InitializeOpenGL();
		void SetApplicationContext(GLFWwindow* window) { m_ApplicationContext = window; }
		void ToggleDepthTesting(bool value);

	private:
		GLFWwindow* m_ApplicationContext;
	};
}
