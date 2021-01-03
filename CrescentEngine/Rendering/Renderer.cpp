#include "CrescentPCH.h";
#include "Renderer.h";
#include <stack>
#include "Scene/Scene.h"


namespace Crescent
{
	Renderer::Renderer()
	{

	}

	Renderer::~Renderer()
	{

	}

	void Renderer::InitializeRenderer()
	{
		//Initialize render items.
		m_RenderCommandBuffer = new CommandBuffer(this);

		//Configure the default OpenGL State.
		m_GLStateCache.ToggleDepthTesting(true);
		m_GLStateCache.ToggleFaceCulling(true);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //Seams are artifacts that shown at times when cubemaps are connected. This flag removes them for seamless connecting through interpolation.
	
		glViewport(0.0f, 0.0f, m_RenderViewportSize.x, m_RenderViewportSize.y);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepth(1.0f); //Specifies the depth value used by glClear to clear the depth buffer.

		m_NDCPlane = new Quad;
		glGenFramebuffers(1, &m_CubemapFramebufferID);
		glGenRenderbuffers(1, &m_CubemapDepthRenderbufferID);

		m_CustomRenderTarget = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, true);
		m_PostProcessingRenderTarget1 = new RenderTarget(1, 1, GL_UNSIGNED_BYTE, 1, false);
		//m_PostProcessor = new PostProcessor(this);

		//Lights
		m_DebugLightMesh = new Sphere(16, 16);
		m_DeferredPointLightMesh = new Sphere(16, 16);

		//Deferred Renderer
		m_GBuffer = new RenderTarget(1, 1, GL_HALF_FLOAT, 4, true);

		//Materials
		m_MaterialLibrary = new MaterialLibrary(m_GBuffer);

		//Shadows
		for (int i = 0; i < 4; ++i) //Allows up to a total of 4 directional/spot shadow casters.
		{
			RenderTarget* renderTarget = new RenderTarget(2048, 2048, GL_UNSIGNED_BYTE, 1, true);
			renderTarget->RetrieveDepthAndStencilAttachment()->BindTexture();
			renderTarget->RetrieveDepthAndStencilAttachment()->SetMinificationFilter(GL_NEAREST);
			renderTarget->RetrieveDepthAndStencilAttachment()->SetMagnificationFilter(GL_NEAREST);
			renderTarget->RetrieveDepthAndStencilAttachment()->SetWrappingMode(GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			m_ShadowRenderTargets.push_back(renderTarget);
		}

		//PBR
		m_PBR = new PBR(this);

		//Uniform Buffer Object
		glGenBuffers(1, &m_GlobalUniformBufferID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUniformBufferID);
		glBufferData(GL_UNIFORM_BUFFER, 720, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_GlobalUniformBufferID);
		
		//Default PBR Precompute (Get a more default oriented HDR Map for this.
		Texture2D* hdrMap = Resources::LoadHDRTexture("Sky Environment", "Resources/Textures/Backgrounds/Alley.hdr");
		PBRCapture* environmentBridge = m_PBR->ProcessEquirectangular(hdrMap);
		SetSkyCapture(environmentBridge);
	}

	void Renderer::SetRenderViewportSize(unsigned int newWidth, unsigned int newHeight)
	{
		m_RenderViewportSize.x = newWidth;
		m_RenderViewportSize.y = newHeight;

		m_GBuffer->ResizeRenderTarget(newWidth, newHeight);

		m_CustomRenderTarget->ResizeRenderTarget(newWidth, newHeight);
		m_PostProcessingRenderTarget1->ResizeRenderTarget(newWidth, newHeight);

		//m_PostProcessor->UpdateRenderSize(newWidth, newHeight);
	}

	glm::vec2 Renderer::RetrieveRenderViewportSize() const
	{
		return m_RenderViewportSize;
	}

	void Renderer::SetRenderTarget(RenderTarget* renderTarget, GLenum target)
	{
		m_CurrentCustomRenderTarget = renderTarget;
		if (renderTarget != nullptr)
		{
			if (std::find(m_CustomRenderTargets.begin(), m_CustomRenderTargets.end(), renderTarget) == m_CustomRenderTargets.end()) //If the render target isn't found in our list...
			{
				m_CustomRenderTargets.push_back(renderTarget);
			}
		}
	}

	Camera* Renderer::RetrieveCamera()
	{
		return m_Camera;
	}

	void Renderer::SetCamera(Camera* camera)
	{
		m_Camera = camera;
	}

	PostProcessor* Renderer::RetrievePostProcessor()
	{
		return m_PostProcessor;
	}

	Material* Renderer::CreateMaterial(const std::string& base)
	{
		return m_MaterialLibrary->CreateMaterial(base);
	}

	Material* Renderer::CreateCustomMaterial(Shader* shader)
	{
		return m_MaterialLibrary->CreateCustomMaterial(shader);
	}

	Material* Renderer::CreatePostProcessingMaterial(Shader* shader)
	{
		return m_MaterialLibrary->CreatePostProcessingMaterial(shader);
	}

	void Renderer::PushRenderCommand(Mesh* mesh, Material* material, glm::mat4 transform, glm::mat4 previousFrameTransform)
	{
		//Get current render target.
		RenderTarget* renderTarget = GetCurrentRenderTarget();
		//Don't render right away but push to the conmmand buffer for later rendering.
		m_RenderCommandBuffer->PushToCommandBuffer(mesh, material, transform, previousFrameTransform, glm::vec3(-99999.0f), glm::vec3(99999.0f), renderTarget);
	}

	void Renderer::PushRenderCommand(SceneNode* node)
	{
		//Update transform before pushing node to render command buffer.
		node->UpdateTransform(true);

		//Get current render target.
		RenderTarget* renderTarget = GetCurrentRenderTarget();

		//Traverse through all the scene nodes and for each node: push its render state to the command buffer together with a calculated transform matrix.
		std::stack<SceneNode*> nodeStack;
		nodeStack.push(node);

		for (unsigned int i = 0; i < node->GetChildNodeCount(); ++i)
		{
			nodeStack.push(node->GetChildByIndex(i));
		}

		while (!nodeStack.empty())
		{
			SceneNode* node = nodeStack.top();
			nodeStack.pop();
			//Only push render command if the child isn't a container node.
			if (node->m_Mesh)
			{
				glm::vec3 boxMinimumWorld = node->GetWorldPosition() + (node->GetWorldScale() * node->m_BoundingBoxMinimum);
				glm::vec3 boxMaximumWorld = node->GetWorldPosition() + (node->GetWorldScale() * node->m_BoundingBoxMaximum);
				m_RenderCommandBuffer->PushToCommandBuffer(node->m_Mesh, node->m_Material, node->GetTransform(), node->GetPreviousTransform(), boxMinimumWorld, boxMaximumWorld, renderTarget);
 			}
			for (unsigned int i = 0; i < node->GetChildNodeCount(); ++i)
			{
				nodeStack.push(node->GetChildByIndex(i));
			}
		}
	}

	void Renderer::PushPostProcessingCommand(Material* postprocessingMaterial)
	{
		//We only care about the material, mesh as NDC quad is pre-defined.
		m_RenderCommandBuffer->PushToCommandBuffer(nullptr, postprocessingMaterial);
	}

	void Renderer::AddLight(DirectionalLight* directionalLight)
	{
		m_DirectionalLights.push_back(directionalLight);
	}

	void Renderer::AddLight(PointLight* pointLight)
	{
		m_PointLights.push_back(pointLight);
	}

	void Renderer::RenderCommandQueueObjects()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		/*  General outline of all the render steps/passes.
		* 
			- First we render all pushed geometry to the GBuffer.
			- We then render all shadow casting geometry to the shadow buffer.
			- Pre-lighting post-processing steps.
			- Deferred lighting pass (ambient, directional, point).
			- Process deferred data so that forward pass is neatly integrated with deferred pass.
			- Then we do the forward "custom" rendering pass where developers can write their own shaders and render stuff as they want, not sacrificing flexibility; this includes
			custom render targets. 
			- Then, we render all alpha-blended materials last.
			- Then we do post-processing, one can supply their own-processing materials by setting the post-processing flag of the material. Each material flagged as post-processing
			will run after the default post-processing shaders (before/after HDR-tonemapping/gamma-correction).
		*/
		
		//Sort all pushed render commands by heavy state-switches (example: shader switches).
		m_RenderCommandBuffer->SortCommandBuffer();

		//Update (global) uniform buffers.
		UpdateGlobalUniformBufferObjects();

		//Set Default GL States.
		m_GLStateCache.ToggleBlending(false);
		m_GLStateCache.ToggleFaceCulling(true);
		m_GLStateCache.SetCullFace(GL_BACK);
		m_GLStateCache.ToggleDepthTesting(true);
		m_GLStateCache.SetDepthTestingFunction(GL_LESS);

		//1) Geometry Buffer
		std::vector<RenderCommand> deferredRenderCommands = m_RenderCommandBuffer->GetDeferredRenderCommand(true);
		glViewport(0, 0, m_RenderViewportSize.x, m_RenderViewportSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer->m_RenderTargetID);
		unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, attachments);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_GLStateCache.SetPolygonMode(m_WireframesEnabled ? GL_LINE : GL_FILL);

		for (unsigned int i = 0; i < deferredRenderCommands.size(); ++i)
		{
			RenderCustomCommand(&deferredRenderCommands[i], nullptr, false);
		}

		m_GLStateCache.SetPolygonMode(GL_FILL);

		attachments[1] = GL_NONE;
		attachments[2] = GL_NONE;
		attachments[3] = GL_NONE;
		glDrawBuffers(4, attachments);
		
		//2) Render all shadow casters to light shadow buffers.
		if (m_ShadowsEnabled)
		{
			m_GLStateCache.SetCullFace(GL_FRONT); //To avoid peterpanning.
			std::vector<RenderCommand> shadowRenderCommands = m_RenderCommandBuffer->GetShadowCastingRenderCommands();
			m_ShadowViewProjectionMatrixes.clear();

			unsigned int shadowRenderTargetIndex = 0;
			for (int i = 0; i < m_DirectionalLights.size(); ++i)
			{
				DirectionalLight* light = m_DirectionalLights[i];
				if (light->m_ShadowCastingEnabled)
				{
					m_MaterialLibrary->m_DirectionalShadowShader->UseShader();

					glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowRenderTargets[shadowRenderTargetIndex]->m_RenderTargetID);
					glViewport(0, 0, m_ShadowRenderTargets[shadowRenderTargetIndex]->m_RenderTargetWidth, m_ShadowRenderTargets[shadowRenderTargetIndex]->m_RenderTargetHeight);
					glClear(GL_DEPTH_BUFFER_BIT);

					glm::mat4 lightProjection = glm::ortho(-20.f, 20.f, 20.0f, -20.0f, -15.0f, 20.0f);
					glm::mat4 lightView = glm::lookAt(-light->m_Direction * 10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					m_DirectionalLights[i]->m_LightSpaceViewProjectionMatrix = lightProjection * lightView;
					m_DirectionalLights[i]->m_ShadowMapRenderTarget = m_ShadowRenderTargets[shadowRenderTargetIndex];

					for (int j = 0; j < shadowRenderCommands.size(); ++j)
					{
						RenderShadowCastCommand(&shadowRenderCommands[j], lightProjection, lightView);
					}
					++shadowRenderTargetIndex;
				}
			}
			m_GLStateCache.SetCullFace(GL_BACK);
		}
		attachments[0] = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(4, attachments);

		//3) Do post-processing before lighting stage (e.g. SSAO).

		//4) Render deferred shader for each light (full quad for direction, spheres for point lights. 
		m_GLStateCache.ToggleDepthTesting(false);
		m_GLStateCache.ToggleBlending(true);
		m_GLStateCache.SetBlendingFunction(GL_ONE, GL_ONE);

		//Ambient Lighting
		RenderDeferredAmbientLighting();

		if (m_LightsEnabled)
		{
			//Directional Light
			for (auto iterator = m_DirectionalLights.begin(); iterator != m_DirectionalLights.end(); ++iterator)
			{
				RenderDeferredDirectionalLighting(*iterator);
			}

			//Point Lights
			m_GLStateCache.SetCullFace(GL_FRONT);
			for (auto iterator = m_PointLights.begin(); iterator != m_PointLights.end(); ++iterator)
			{
				//Only render point lights if within frustrum.
				if (m_Camera->m_CameraFrustrum.Intersect((*iterator)->m_Position, (*iterator)->m_Radius))
				{
					RenderDeferredPointLighting(*iterator);
				}
			}
			m_GLStateCache.SetCullFace(GL_BACK);
		}

		m_GLStateCache.ToggleDepthTesting(true);
		m_GLStateCache.SetBlendingFunction(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		m_GLStateCache.ToggleBlending(false);

		//5) Blit depth render to default for forward rendering.
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->m_RenderTargetID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_CustomRenderTarget->m_RenderTargetID);
		glBlitFramebuffer(0, 0, m_GBuffer->m_RenderTargetWidth, m_GBuffer->m_RenderTargetHeight, 0, 0, m_RenderViewportSize.x, m_RenderViewportSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		//6) Custom forward render pass. Push default render target to the end of the render target buffer so that we always render the default buffer last. 
		m_CustomRenderTargets.push_back(nullptr);
		for (unsigned int targetIndex = 0; targetIndex < m_CustomRenderTargets.size(); ++targetIndex)
		{
			RenderTarget* renderTarget = m_CustomRenderTargets[targetIndex];
			if (renderTarget)
			{
				glViewport(0, 0, renderTarget->m_RenderTargetWidth, renderTarget->m_RenderTargetHeight);
				glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->m_RenderTargetID);
				if (renderTarget->m_RenderTargetHasDepthAndStencil)
				{
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				}
				else
				{
					glClear(GL_COLOR_BUFFER_BIT);
				}
				m_Camera->SetPerspective(m_Camera->m_FieldOfView, (float)renderTarget->m_RenderTargetWidth / (float)renderTarget->m_RenderTargetHeight, 0.1f, 100.0f);
			}
			else
			{
				//Don't render to default framebuffer, but to custom framebuffer which we will use for postprocessing.
				glViewport(0, 0, m_RenderViewportSize.x, m_RenderViewportSize.y);
				glBindFramebuffer(GL_FRAMEBUFFER, m_CustomRenderTarget->m_RenderTargetID);
				m_Camera->SetPerspective(m_Camera->m_FieldOfView, m_RenderViewportSize.x / m_RenderViewportSize.y, 0.1, 100.0f);
			}

			//Sort all render commands and retrieving the sort arrays.
			std::vector<RenderCommand> renderCommands = m_RenderCommandBuffer->GetCustomRenderCommands(renderTarget);

			//Iterate over the render commands and execute.
			m_GLStateCache.SetPolygonMode(m_WireframesEnabled ? GL_LINE : GL_FILL);
			for (unsigned int i = 0; i < renderCommands.size(); ++i)
			{
				RenderCustomCommand(&renderCommands[i], nullptr);
			}
			m_GLStateCache.SetPolygonMode(GL_FILL);
		}

		//7) Alpha Material Pass
		glViewport(0, 0, m_RenderViewportSize.x, m_RenderViewportSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_CustomRenderTarget->m_RenderTargetID);
		std::vector<RenderCommand> alphaRenderCommands = m_RenderCommandBuffer->GetAlphaRenderCommands();
		for (unsigned int i = 0; i < alphaRenderCommands.size(); ++i)
		{
			RenderCustomCommand(&alphaRenderCommands[i], nullptr);
		}

		//Render light mesh (as visual cue), if requested.
		for (auto iterator = m_PointLights.begin(); iterator != m_PointLights.end(); ++iterator)
		{
			if ((*iterator)->m_MeshRenderEnabled)
			{
				m_MaterialLibrary->m_DebugLightMaterial->SetShaderVector3("lightColor", (*iterator)->m_Color * (*iterator)->m_Intensity * 0.25f);

				RenderCommand renderCommand;

				renderCommand.m_Material = m_MaterialLibrary->m_DebugLightMaterial;
				renderCommand.m_Mesh = m_DebugLightMesh;
				glm::mat4 modelMatrix = glm::mat4(1.0f);
				modelMatrix = glm::translate(modelMatrix, (*iterator)->m_Position);
				modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f));
				renderCommand.m_Transform = modelMatrix;

				RenderCustomCommand(&renderCommand, nullptr);
			}
		}

		//8) Post-Processing stage after all lighting calculations.
		//m_PostProcessor->ProcessPostLighting()

		//9) Render Debug Visuals
		glViewport(0, 0, m_RenderViewportSize.x, m_RenderViewportSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_CustomRenderTarget->m_RenderTargetID);
		if (m_LightVolumesEnabled)
		{
			m_GLStateCache.SetPolygonMode(GL_LINE);
			m_GLStateCache.SetCullFace(GL_FRONT);
			for (auto iterator = m_PointLights.begin(); iterator != m_PointLights.end(); ++iterator)
			{
				m_MaterialLibrary->m_DebugLightMaterial->SetShaderVector3("lightColor", (*iterator)->m_Color);

				RenderCommand renderCommand;
				renderCommand.m_Material = m_MaterialLibrary->m_DebugLightMaterial;
				renderCommand.m_Mesh = m_DebugLightMesh;
				glm::mat4 modelMatrix = glm::mat4(1.0f);
				modelMatrix = glm::translate(modelMatrix, (*iterator)->m_Position);
				modelMatrix = glm::scale(modelMatrix, glm::vec3((*iterator)->m_Radius));
				renderCommand.m_Transform = modelMatrix;

				RenderCustomCommand(&renderCommand, nullptr);
			}
			m_GLStateCache.SetPolygonMode(GL_FILL);
			m_GLStateCache.SetCullFace(GL_BACK);
		}
		if (m_RenderProbesEnabled)
		{
			m_PBR->RenderProbes();
		}
		
		//10) Custom Post-Processing Pass
		std::vector<RenderCommand> postProcessingCommands = m_RenderCommandBuffer->GetPostProcessingRenderCommands();
		for (unsigned int i = 0; i < postProcessingCommands.size(); ++i)
		{
			//Ping-pong between render textures.
			bool even = i % 2 == 0;
			Blit(even ? m_CustomRenderTarget->RetrieveColorAttachment(0) : m_PostProcessingRenderTarget1->RetrieveColorAttachment(0),
				even ? m_PostProcessingRenderTarget1 : m_CustomRenderTarget, postProcessingCommands[i].m_Material);
		}

		//11) Final post-processing steps, blitting to default framebuffer.
		

	}

	void Renderer::Blit(Texture2D* source, RenderTarget* destination, Material* material, std::string textureUniformName)
	{
	}

	void Renderer::SetSkyCapture(PBRCapture* pbrEnvironment)
	{
		m_PBR->SetSkyCapture(pbrEnvironment);
	}

	PBRCapture* Renderer::GetSkyCapture()
	{
		return m_PBR->GetSkyCapture();
	}

	void Renderer::AddIrradianceProbe(glm::vec3 position, float radius)
	{
		m_ProbeSpatials.push_back(glm::vec4(position, radius));
	}

	void Renderer::BakeProbes(SceneNode* scene)
	{
		if (!scene)
		{
			//If no scene node was provided, use root node. (Capture All)
			scene = Scene::m_RootNode;
		}
		scene->UpdateTransform();
		//Build a command list of nodes within the reflection probe's capture box/radius.
		CommandBuffer commandBuffer(this);
		std::vector<Material*> materials;

	}















	void Renderer::InitializeOpenGL()
	{
		if (glewInit() != GLEW_OK)
		{
			CrescentError("Failed to initialize GLEW.")
		}
		CrescentInfo("Successfully initialized GLEW.");
	}

	void Renderer::ToggleDepthTesting(bool value)
	{
		value ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	}

	void Renderer::ToggleWireframeRendering(bool value)
	{
		value ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void Renderer::ToggleBlending(bool value)
	{
		value ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void Renderer::ToggleFaceCulling(bool value)
	{
		value ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	}

	void Renderer::RenderCustomCommand(RenderCommand* renderCommand, Camera* customCamera, bool updateGLSettings)
	{
	}

	void Renderer::RenderMesh(Mesh* mesh, Shader* shader)
	{
	}

	void Renderer::UpdateGlobalUniformBufferObjects()
	{
	}

	RenderTarget* Renderer::GetCurrentRenderTarget()
	{
		return nullptr;
	}

	void Renderer::RenderDeferredAmbientLighting()
	{
	}

	void Renderer::RenderDeferredDirectionalLighting(DirectionalLight* directionalLight)
	{
	}

	void Renderer::RenderDeferredPointLighting(PointLight* pointLight)
	{
	}

	void Renderer::RenderShadowCastCommand(RenderCommand* renderCommand, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
	{
	}

	void Renderer::ClearBuffers()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}