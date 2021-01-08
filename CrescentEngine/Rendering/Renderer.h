#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "RenderCommand.h"

namespace Crescent
{
	class SceneEntity;
	class RenderQueue;
	class Shader;
	class GLStateCache;
	class Mesh;
	class Camera;
	class Material;
	class MaterialLibrary;
	class RenderTarget;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void InitializeRenderer(const int& renderWindowWidth, const int& renderWindowHeight);

		//Rendering Items
		void PushToRenderQueue(SceneEntity* sceneEntity);
		void RenderAllQueueItems();

		//Window Size
		void SetRenderingWindowSize(const int& newWidth, const int& newHeight);

		//Camera
		void SetSceneCamera(Camera* sceneCamera);
		Camera* RetrieveSceneCamera();

		//Creation
		Material* CreateMaterial(std::string shaderName = "Default"); //Default materials. These materials have default state and uses checkboard texture as its albedo/diffuse (and black metalliic, half roughness purple normals and white AO).

		//Render Target
		void SetCurrentRenderTarget(RenderTarget* renderTarget, GLenum framebufferTarget);

		//Retrieve
		const char* RetrieveDeviceRendererInformation() const { return m_DeviceRendererInformation; }
		const char* RetrieveDeviceVendorInformation() const { return m_DeviceVendorInformation; }
		const char* RetrieveDeviceVersionInformation() const { return m_DeviceVersionInformation; }
		glm::vec2 RetrieveRenderWindowSize() const { return m_RenderWindowSize; }
		GLStateCache* RetrieveGLStateCache() { return m_GLStateCache; }
		RenderTarget* RetrieveCurrentRenderTarget();
		RenderTarget* RetrieveMainRenderTarget();

	private:
		//Renderer-specific logic for rendering a custom forward-pass command.
		void RenderForwardPassCommand(RenderCommand* renderCommand, Camera* customRenderCamera, bool updateGLStates = true);
		void RenderMesh(Mesh* mesh);

	private:
		MaterialLibrary* m_MaterialLibrary = nullptr;
		RenderQueue* m_RenderQueue = nullptr;
		GLStateCache* m_GLStateCache = nullptr;
		Camera* m_Camera = nullptr;

		//Render Targets
		RenderTarget* m_CurrentCustomRenderTarget = nullptr;
		RenderTarget* m_MainRenderTarget = nullptr;

		//Driver Information
		const char* m_DeviceRendererInformation = nullptr;
		const char* m_DeviceVendorInformation = nullptr;
		const char* m_DeviceVersionInformation = nullptr;

		glm::vec2 m_RenderWindowSize = glm::vec2(0.0f);
	};
}
