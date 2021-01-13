#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "RenderCommand.h"
#include "EnvironmentalPBR.h"
#include "PBR.h"

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
	class DirectionalLight;
	class PointLight;
	class Quad;
	class Texture;
	class TextureCube;
	class EnvironmentalPBR;
	class PBR;

	class Renderer
	{
		friend PBR;

	public:
		Renderer();
		~Renderer();

		void InitializeRenderer(const int& renderWindowWidth, const int& renderWindowHeight, Camera* sceneCamera);

		//Rendering Items
		void PushToRenderQueue(SceneEntity* sceneEntity);
		void RenderAllQueueItems();

		//Window Size
		void SetRenderingWindowSize(const int& newWidth, const int& newHeight);

		//Blitting
		void Blit(Texture* textureSource, RenderTarget* targetDestination = nullptr, Material* material = nullptr, std::string textureUniformName = "TexSrc");

		//Camera
		void SetSceneCamera(Camera* sceneCamera);
		Camera* RetrieveSceneCamera();

		//Creation
		Material* CreateMaterial(std::string shaderName = "Default"); //Default materials. These materials have default state and uses checkboard texture as its albedo/diffuse (and black metalliic, half roughness purple normals and white AO).
		void AddLightSource(DirectionalLight* directionalLight);
		void AddLightSource(PointLight* pointLight);

		//Retrieve
		EnvironmentalPBR* RetrieveSkyCapture();
		void SetSkyCapture(EnvironmentalPBR* capturedEnvironment);

		//Cubemap
		void RenderCubemap(SceneEntity* sceneEntity, TextureCube* cubemapTarget, glm::vec3 position = glm::vec3(0.0f), unsigned int mipmappingLevel = 0);
		void RenderCubemap(std::vector<RenderCommand>& renderCommands, TextureCube* cubeTarget, glm::vec3 position = glm::vec3(0.0f), unsigned int mipmappingLevel = 0);

		const char* RetrieveDeviceRendererInformation() const { return m_DeviceRendererInformation; }
		const char* RetrieveDeviceVendorInformation() const { return m_DeviceVendorInformation; }
		const char* RetrieveDeviceVersionInformation() const { return m_DeviceVersionInformation; }
		glm::vec2 RetrieveRenderWindowSize() const { return m_RenderWindowSize; }

		GLStateCache* RetrieveGLStateCache() { return m_GLStateCache; }

		RenderTarget* RetrieveMainRenderTarget();
		RenderTarget* RetrieveGBuffer();
		RenderTarget* RetrieveShadowRenderTarget(int index = 0);
		RenderTarget* RetrieveCustomRenderTarget();

	public:
		bool m_ShadowsEnabled = true;
		bool m_LightsEnabled = true;
		bool m_ShowDebugLightVolumes = true;
		bool m_WireframesEnabled = false;
		bool m_CubemapEnabled = true;

	private:
		//Renderer-specific logic for rendering a custom forward-pass command.
		void RenderCustomCommand(RenderCommand* renderCommand, Camera* customRenderCamera, bool updateGLStates = true);
		void RenderMesh(Mesh* mesh);

		//Render Directional Light
		void RenderDeferredDirectionalLight(DirectionalLight* directionalLight);
		//Render Point Light
		void RenderDeferredPointLight(PointLight* pointLight);
		
		//Render Mesh for Shadow Buffer Generation
		void RenderShadowCastCommand(RenderCommand* renderCommand, const glm::mat4& lightSpaceProjectionMatrix, const glm::mat4& lightSpaceViewMatrix);

		//Update the global uniform buffer objects.
		void UpdateGlobalUniformBufferObjects();

		//Final
		void BlitToMainFramebuffer(Texture* sourceRenderTarget);

	private:
		//Temporary
		Shader* m_PostProcessShader = nullptr;

		//PBR
		PBR* m_PBR = nullptr;

		//UBO
		unsigned int m_GlobalUniformBufferID;

		MaterialLibrary* m_MaterialLibrary = nullptr;
		RenderQueue* m_RenderQueue = nullptr;
		GLStateCache* m_GLStateCache = nullptr;
		Camera* m_Camera = nullptr;

		//Render Targets
		RenderTarget* m_GBuffer = nullptr;
		RenderTarget* m_CustomRenderTarget = nullptr;
		RenderTarget* m_MainRenderTarget = nullptr;
		RenderTarget* m_PostProcessRenderTarget = nullptr;
		unsigned int m_CubemapFramebufferID;
		unsigned int m_CubemapDepthRenderbufferID;

		std::vector<RenderTarget*> m_RenderTargetsCustom;
		Quad* m_NDCQuad = nullptr;

		//Shadow Target
		std::vector<RenderTarget*> m_ShadowRenderTargets;;

		//Lights
		std::vector<DirectionalLight*> m_DirectionalLights;
		std::vector<PointLight*> m_PointLights;
		Mesh* m_DeferredPointLightMesh = nullptr;

		glm::vec2 m_RenderWindowSize = glm::vec2(0.0f);

		//Driver Information
		const char* m_DeviceRendererInformation = nullptr;
		const char* m_DeviceVendorInformation = nullptr;
		const char* m_DeviceVersionInformation = nullptr;

		//Debug
		Mesh* m_DebugLightMesh = nullptr;

	};
}
