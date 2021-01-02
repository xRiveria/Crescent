#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../Rendering/Material.h"
#include "PBRCapture.h"

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
		void ToggleFaceCulling(bool value);
		
		Material* CreateMaterial();
		Material* CreateMaterial(const std::string& base);
		Material* CreateCustomMaterial(Shader* shader);
		Material* CreatePostProcessingMaterial(Shader* shader);
		PBRCapture* GetSkyCapture();

		void ClearBuffers();

	private:
		GLFWwindow* m_ApplicationContext = nullptr;
	};
}
