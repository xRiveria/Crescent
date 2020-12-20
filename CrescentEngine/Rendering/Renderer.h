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
		void ToggleWireframeRendering(bool value);
		void ToggleBlending(bool value);

		void ClearBuffers();

	private:
		GLFWwindow* m_ApplicationContext = nullptr;
	};
}
