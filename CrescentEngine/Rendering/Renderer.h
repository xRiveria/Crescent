#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../Rendering/Material.h"
#include "PBRCapture.h"
#include "RenderQueue.h"
#include "RenderTarget.h"
#include "CommandBuffer.h"
#include "GLStateCache.h"
#include "../Lighting/Lights.h"
#include "MaterialLibrary.h"
#include "../Utilities/Camera.h"
#include "../Models/DefaultShapes.h"
#include "PostProcessor.h"
#include "PBR.h"
#include "Scene/SceneNode.h"
#include "../Memory/Resources.h"

namespace Crescent
{
	/*
		Our main renderer. This is responsible for maintaining a render buffer queue, providing the front push commands for filling the buffer, sorting the buffer, manage multiple
		render passes and render the buffer(s) accordingly.
	*/

	class Renderer
	{
	public:
		Renderer();
		~Renderer();
		
		//===============================================================================================================
		void InitializeRenderer();
		void SetRenderViewportSize(unsigned int newWidth, unsigned int newHeight);
		glm::vec2 RetrieveRenderViewportSize() const;

		void SetRenderTarget(RenderTarget* renderTarget, GLenum target = GL_TEXTURE_2D);
		Camera* RetrieveCamera();
		void SetCamera(Camera* camera);

		PostProcessor* RetrievePostProcessor();

		//Create either a deffered defualt material (based on a default set of materials avaliable (like glass), or a custom material (with custom, you have to supply your own shader).
		Material* CreateMaterial(const std::string& base = "Default"); //These don't have the custom flag set (default material has default state and uses checkerboard texture as Albedo (and black metallic, half roughness, purple normals, white AO).
		Material* CreateCustomMaterial(Shader* shader); //These have the custom flag set and will be rendered in the forward pass.
		Material* CreatePostProcessingMaterial(Shader* shader); //These have the post-processing flag set (will be rendered after deferred/forward pass). 

		//Render Queue Pushing
		void PushRenderCommand(Mesh* mesh, Material* material, glm::mat4 transform = glm::mat4(1.0f), glm::mat4 previousFrameTransform = glm::mat4(1.0f));
		void PushRenderCommand(SceneNode* node);
		void PushPostProcessingCommand(Material* postprocessingMaterial);

		//Lights
		void AddLight(DirectionalLight* directionalLight);
		void AddLight(PointLight* pointLight);

		void RenderCommandQueueObjects();

		void Blit(Texture2D* source, RenderTarget* destination = nullptr, Material* material = nullptr, std::string textureUniformName = "TexSrc");

		//PBR
		void SetSkyCapture(PBRCapture* pbrEnvironment);
		PBRCapture* GetSkyCapture();
		void AddIrradianceProbe(glm::vec3 position, float radius);
		void BakeProbes(SceneNode* scene = nullptr);

		//Cleanup.
		void InitializeOpenGL();
		void SetApplicationContext(GLFWwindow* window) { m_ApplicationContext = window; }
		void ClearBuffers();
		void ToggleDepthTesting(bool value);
		void ToggleWireframeRendering(bool value);
		void ToggleBlending(bool value);
		void ToggleFaceCulling(bool value);

	private:
		//Renderer specific logic for rendering a custom (forward-pass) command.
		void RenderCustomCommand(RenderCommand* renderCommand, Camera* customCamera, bool updateGLSettings = true);

		//Minimal logic to render a mesh.
		void RenderMesh(Mesh* mesh, Shader* shader);
		//Updates the global uniform buffer objects.
		void UpdateGlobalUniformBufferObjects();
		//Returns the currently active render target.
		RenderTarget* GetCurrentRenderTarget();

		//Deferred Logic.
		void RenderDeferredAmbientLighting(); //Renders all ambient lighting including indirect IBL.
		void RenderDeferredDirectionalLighting(DirectionalLight* directionalLight);
		void RenderDeferredPointLighting(PointLight* pointLight);

		//Render mesh for shadow buffer generation.
		void RenderShadowCastCommand(RenderCommand* renderCommand, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);

	public:
		bool m_ShadowsEnabled = true;
		bool m_LightsEnabled = true;
		bool m_RenderLightsEnabled = true;
		bool m_LightVolumesEnabled = false;
		bool m_RenderProbesEnabled = false;
		bool m_WireframesEnabled = false;

	private:
		CommandBuffer* m_RenderCommandBuffer;
		GLStateCache m_GLStateCache;
		glm::vec2 m_RenderViewportSize;

		//Lighting
		std::vector<DirectionalLight*> m_DirectionalLights;
		std::vector<PointLight*> m_PointLights;
		RenderTarget* m_GBuffer = nullptr;
		Mesh* m_DeferredPointLightMesh;

		//Materials
		MaterialLibrary* m_MaterialLibrary;

		//Camera
		Camera* m_Camera;
		glm::mat4 m_PreviousViewProjectionMatrix;

		//Render-Targets/Post-Processing
		std::vector<RenderTarget*> m_CustomRenderTargets;
		RenderTarget* m_CurrentCustomRenderTarget = nullptr;
		RenderTarget* m_CustomRenderTarget;
		RenderTarget* m_PostProcessingRenderTarget1;
		PostProcessor* m_PostProcessor;
		Quad* m_NDCPlane;
		unsigned int m_CubemapFramebufferID;
		unsigned int m_CubemapDepthRenderbufferID;

		//Shadow Buffers
		std::vector<RenderTarget*> m_ShadowRenderTargets;
		std::vector<glm::mat4> m_ShadowViewProjectionMatrixes;

		//PBR
		PBR* m_PBR;
		unsigned int m_PBREnvironmentIndex;
		std::vector<glm::vec4> m_ProbeSpatials;

		//UBO
		unsigned int m_GlobalUniformBufferID;

		//Debug
		Mesh* m_DebugLightMesh;

	private:
		GLFWwindow* m_ApplicationContext = nullptr;
	};
}
