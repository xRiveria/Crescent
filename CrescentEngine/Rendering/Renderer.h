#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../Rendering/Material.h"
#include "PBRCapture.h"
#include "RenderQueue.h"
#include "RenderTarget.h"
#include "GLStateCache.h"

namespace Crescent
{
	/*
		Our main renderer. This is responsible for maintaining a render buffer queue, providing the front push commands for filling the buffer, sorting the buffer, manage multiple
		render passes and render the buffer(s) accordingly.
	*/

	class Renderer
	{
	public:
		Renderer() {}
		~Renderer();

		void InitializeOpenGL();
		void SetApplicationContext(GLFWwindow* window) { m_ApplicationContext = window; }
		void ClearBuffers();

		//Cleanup.
		void ToggleDepthTesting(bool value);
		void ToggleWireframeRendering(bool value);
		void ToggleBlending(bool value);
		void ToggleFaceCulling(bool value);
		

		Material* CreateMaterial();
		Material* CreateMaterial(const std::string& base);
		Material* CreateCustomMaterial(Shader* shader);
		Material* CreatePostProcessingMaterial(Shader* shader);
		PBRCapture* GetSkyCapture();

	public:
		bool m_ShadowsEnabled = true;
		bool m_LightsEnabled = true;
		bool m_RenderLightsEnabled = true;
		bool m_LightVolumesEnabled = false;
		bool m_RenderProbesEnabled = false;
		bool m_WireframesEnabled = false;

	private:
		RenderQueue* m_RenderQueue;
		GLStateCache m_GLStateCache;
		glm::vec2 m_ViewportSize;


	private:
		GLFWwindow* m_ApplicationContext = nullptr;
	};
}
