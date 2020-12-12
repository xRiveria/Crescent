#pragma once
#include "imgui/imgui.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"

namespace CrescentEngine
{
	class Editor
	{
	public:
		Editor();
		Editor(GLFWwindow* applicationContext);

		void SetApplicationContext(GLFWwindow* window);
		void InitializeImGui();
		void BeginEditorRenderLoop();
		void EndEditorRenderLoop();
		void SetEditorDarkThemeColors();

	private:
		GLFWwindow* m_ApplicationContext;
	};
}
