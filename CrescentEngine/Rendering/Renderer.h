#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Crescent
{
	class SceneEntity;
	class RenderQueue;
	class Shader;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void InitializeRenderer();

		//Rendering Items
		void PushToRenderQueue(SceneEntity* sceneEntity);

		void RenderAllQueueItems(Shader* temporaryShader);

		void InitializeOpenGL();
		void SetApplicationContext(GLFWwindow* window) { m_ApplicationContext = window; }
		void ToggleDepthTesting(bool value);
		void ToggleWireframeRendering(bool value);
		void ToggleBlending(bool value);
		void ToggleFaceCulling(bool value);

		void ClearBuffers();

	private:
		RenderQueue* m_RenderQueue = nullptr;
		GLFWwindow* m_ApplicationContext = nullptr;
	};
}
