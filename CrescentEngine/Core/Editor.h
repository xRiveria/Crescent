#pragma once
#include "imgui/imgui.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "../Core/Window.h"

namespace CrescentEngine
{
	class Editor
	{
	public:
		Editor();
		Editor(Window* applicationContext);

		void SetApplicationContext(Window* applicationContext);
		void InitializeImGui();

		void BeginEditorRenderLoop();
		void RenderDockingContext(); //Create all user menus below. 
		void EndEditorRenderLoop();

		void SetEditorDarkThemeColors();
		void SetViewportSize(int newWidth, int newHeight);
		int RetrieveViewportWidth() const { return m_ViewportWidth; }
		int RetrieveViewportHeight() const { return m_ViewportHeight; }

	private:
		Window* m_ApplicationContext;
		int m_ViewportWidth = 0.0f;
		int m_ViewportHeight = 0.0f;
	};
}
