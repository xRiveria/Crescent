#include "CrescentPCH.h";
#include "Renderer.h";
#include "RenderQueue.h"
#include "GLStateCache.h"
#include "MaterialLibrary.h"
#include "../Models/DefaultPrimitives.h"
#include "../Utilities/FlyCamera.h"
#include "../Scene/SceneEntity.h"
#include "../Models/Model.h"
#include "../Shading/Shader.h"
#include "../Shading/Material.h"
#include "../Lighting/DirectionalLight.h"
#include "../Lighting/PointLight.h"
#include "../Rendering/Resources.h"
#include "../Shading/TextureCube.h"
#include "PostProcessor.h"
#include <glm/gtc/type_ptr.hpp>
#include <stack>

//As of now, our renderer only supports Forward Pass Rendering.

namespace Crescent
{
	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
		delete m_RenderQueue;
		delete m_NDCQuad;
		delete m_MaterialLibrary;
		delete m_GBuffer;
		delete m_CustomRenderTarget;

		for (int i = 0; i < m_ShadowRenderTargets.size(); i++)
		{
			delete m_ShadowRenderTargets[i];
		}

		delete m_DebugLightMesh;
		delete m_PostProcessRenderTarget;
		delete m_PostProcessor;
		delete m_PBR;
	}

	void Renderer::InitializeRenderer(const int& renderWindowWidth, const int& renderWindowHeight, Camera* sceneCamera)
	{
		if (glewInit() != GLEW_OK)
		{
			CrescentError("Failed to initialize GLEW.")
		}
		CrescentInfo("Successfully initialized GLEW.");

		//Configure Default OpenGL State
		m_GLStateCache = new GLStateCache();
		m_GLStateCache->ToggleDepthTesting(true);
		m_GLStateCache->ToggleFaceCulling(true);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //For our prefiltered environment maps to prevent seams. 

		m_DeviceRendererInformation = (char*)glGetString(GL_RENDERER);
		m_DeviceVendorInformation = (char*)glGetString(GL_VENDOR);
		m_DeviceVersionInformation = (char*)glGetString(GL_VERSION);
		
		m_RenderWindowSize = glm::vec2(renderWindowWidth, renderWindowHeight);
		glViewport(0.0f, 0.0f, renderWindowWidth, renderWindowHeight);
		glClearDepth(1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		m_Camera = sceneCamera;

		//Core Primitives
		m_NDCQuad = new Quad();
		m_DebugLightMesh = new Sphere(16, 16);
		m_DeferredPointLightMesh = new Sphere(16, 16);

		//Core Systems
		m_RenderQueue = new RenderQueue(this);
		m_MaterialLibrary = new MaterialLibrary(m_GBuffer);

		//Render Targets
		m_MainRenderTarget = new RenderTarget(1, 1, GL_FLOAT, 1, true);
		m_GBuffer = new RenderTarget(1, 1, GL_HALF_FLOAT, 4, true);
		m_CustomRenderTarget = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, true);
		m_PostProcessRenderTarget = new RenderTarget(1, 1, GL_UNSIGNED_BYTE, 1, false);
		m_PostProcessor = new PostProcessor(this);

		//Shadows
		for (int i = 0; i < 4; i++) //Allow for up to a total of 4 directional/spot shadow casters.
		{
			RenderTarget* renderTarget = new RenderTarget(2048, 2048, GL_UNSIGNED_BYTE, 1, true);
			renderTarget->m_DepthAndStencilAttachment.BindTexture();

			renderTarget->m_DepthAndStencilAttachment.SetMinificationFilter(GL_NEAREST);
			renderTarget->m_DepthAndStencilAttachment.SetMagnificationFilter(GL_NEAREST);
			renderTarget->m_DepthAndStencilAttachment.SetWrappingMode(GL_CLAMP_TO_BORDER);

			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			m_ShadowRenderTargets.push_back(renderTarget);
		}

		//Cubemap
		glGenFramebuffers(1, &m_CubemapFramebufferID);
		glGenRenderbuffers(1, &m_CubemapDepthRenderbufferID);

		m_PBR = new PBR(this);

		//Default PBR Pre-Compute (Get a more default oriented HDR map for this).
		//Texture* milkyWayMap = Resources::LoadHDRTexture("Sky Environment", "Resources/Skybox/MilkyWay/Milkyway_small.hdr");
		Texture* milkyWayMap = Resources::LoadHDRTexture("Sky Environment", "Resources/Skybox/AlleyWay/Alley.hdr");
		EnvironmentalPBR* environmentalCapture = m_PBR->ProcessEquirectangularMap(milkyWayMap);
		SetSkyCapture(environmentalCapture);

		//Global Uniform Buffer Object
		//glGenBuffers(1, &m_GlobalUniformBufferID);
		//glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUniformBufferID);
		//glBufferData(GL_UNIFORM_BUFFER, 720, nullptr, GL_STREAM_DRAW);
		//glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_GlobalUniformBufferID);
	}

	void Renderer::PushToRenderQueue(SceneEntity* sceneEntity)
	{
		//Update transform before pushing node to render command buffer.
		sceneEntity->UpdateEntityTransform(true);

		std::stack<SceneEntity*> nodeStack;
		nodeStack.push(sceneEntity);
		for (unsigned int i = 0; i < sceneEntity->RetrieveChildCount(); i++)
		{
			nodeStack.push(sceneEntity->RetrieveChildByIndex(i));
		}

		while (!nodeStack.empty())
		{
			SceneEntity* node = nodeStack.top();
			nodeStack.pop();
			if (node->m_Mesh)
			{
				m_RenderQueue->PushToRenderQueue(node->m_Mesh, node->m_Material, node->RetrieveEntityTransform());
			}

			for (unsigned int i = 0; i < node->RetrieveChildCount(); i++)
			{
				nodeStack.push(node->RetrieveChildByIndex(i));
			}
		}
	}

	//Attach shader to material.
	void Renderer::RenderAllQueueItems()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Update Global Uniform Buffer Object
		UpdateGlobalUniformBufferObjects();

		//Set default OpenGL state.
		m_GLStateCache->ToggleBlending(false);
		m_GLStateCache->ToggleFaceCulling(true);
		m_GLStateCache->SetCulledFace(GL_BACK);
		m_GLStateCache->ToggleDepthTesting(true);
		m_GLStateCache->SetDepthFunction(GL_LESS);
		
		//1) Geometry Buffer
		std::vector<RenderCommand> deferredRenderCommands = m_RenderQueue->RetrieveDeferredRenderingCommands();
		glViewport(0, 0, m_RenderWindowSize.x, m_RenderWindowSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer->m_FramebufferID);
		unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, attachments);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_GLStateCache->SetPolygonMode(m_WireframesEnabled ? GL_LINE : GL_FILL);

		for (int i = 0; i < deferredRenderCommands.size(); i++)
		{
			RenderCustomCommand(&deferredRenderCommands[i], nullptr, false);
		}
		m_GLStateCache->SetPolygonMode(GL_FILL);

		//Disable for next pass (shadow map generation).
		attachments[1] = GL_NONE;
		attachments[2] = GL_NONE;
		attachments[3] = GL_NONE;
		glDrawBuffers(4, attachments);

		//2) Render All Shadow Casters to Light Shadow Buffers
		if (m_ShadowsEnabled)
		{
			m_GLStateCache->SetCulledFace(GL_FRONT);
			std::vector<RenderCommand> shadowRenderCommands = m_RenderQueue->RetrieveShadowCastingRenderCommands();

			unsigned int shadowRenderTargetIndex = 0;
			for (int i = 0; i < m_DirectionalLights.size(); i++) //We usually have 1 directional light source.
			{
				DirectionalLight* directionalLight = m_DirectionalLights[i];
				if (directionalLight->m_ShadowCastingEnabled)
				{
					m_MaterialLibrary->m_DirectionalShadowShader->UseShader();

					glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowRenderTargets[shadowRenderTargetIndex]->m_FramebufferID);
					glViewport(0, 0, m_ShadowRenderTargets[shadowRenderTargetIndex]->m_FramebufferWidth, m_ShadowRenderTargets[shadowRenderTargetIndex]->m_FramebufferHeight);
					glClear(GL_DEPTH_BUFFER_BIT);

					glm::mat4 lightProjectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, -15.0f, 20.0f);
					glm::mat4 lightViewMatrix = glm::lookAt(-directionalLight->m_LightDirection * 10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

					m_DirectionalLights[i]->m_LightSpaceViewProjectionMatrix = lightProjectionMatrix * lightViewMatrix;
					m_DirectionalLights[i]->m_ShadowMapRenderTarget = m_ShadowRenderTargets[shadowRenderTargetIndex];

					//This varies based on the amount of objects in our scene that can cast shadows on objects. This filtered whenever we submit commands into the render queue.
					//By default, all physical objects in the scene can cast and receive shadows.
					for (int j = 0; j < shadowRenderCommands.size(); j++) 
					{
						RenderShadowCastCommand(&shadowRenderCommands[j], lightProjectionMatrix, lightViewMatrix);
					}
					shadowRenderTargetIndex++;
				}
			}
			m_GLStateCache->SetCulledFace(GL_BACK);
		}
		attachments[0] = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(4, attachments);

		//3) Do post-processing steps before lighting stage.
		m_PostProcessor->ProcessPreLighting(this, m_GBuffer, m_Camera);

		//4) Render deferred shader for each light (full quad for directional, spheres for point lights).
		glBindFramebuffer(GL_FRAMEBUFFER, m_CustomRenderTarget->m_FramebufferID);
		glViewport(0, 0, m_CustomRenderTarget->m_FramebufferWidth, m_CustomRenderTarget->m_FramebufferHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		m_GLStateCache->ToggleDepthTesting(false);
		m_GLStateCache->ToggleBlending(true);
		m_GLStateCache->SetBlendingFunction(GL_ONE, GL_ONE);

		//Binds our color buffers to the respective texture slots. Remember that Texture Slot 0 (Position), 1 (Normals) and 2 (Albedo) are always used for our GBuffer outputs. 
		m_GBuffer->RetrieveColorAttachment(0)->BindTexture(0);
		m_GBuffer->RetrieveColorAttachment(1)->BindTexture(1);
		m_GBuffer->RetrieveColorAttachment(2)->BindTexture(2);

		///Ambient Lighting
		RenderDeferredAmbientLight();

		if (m_LightsEnabled)
		{
			//Directional Lights
			for (auto iterator = m_DirectionalLights.begin(); iterator != m_DirectionalLights.end(); iterator++)
			{
				RenderDeferredDirectionalLight(*iterator);
			}
			
			//Point Lights
			m_GLStateCache->SetCulledFace(GL_FRONT);
			for (auto iterator = m_PointLights.begin(); iterator != m_PointLights.end(); iterator++) //Remember that our objects are stored as pointers, thus the dereference.
			{
				///Frustrum Check.
				RenderDeferredPointLight(*iterator);
			}
			m_GLStateCache->SetCulledFace(GL_BACK);
		}

		m_GLStateCache->ToggleDepthTesting(true);
		m_GLStateCache->SetBlendingFunction(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		m_GLStateCache->ToggleBlending(false);

		//5) Blit Depth Framebuffer to Default for Rendering
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->m_FramebufferID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_CustomRenderTarget->m_FramebufferID); //Write to our default framebuffer.
		glBlitFramebuffer(0, 0, m_GBuffer->m_FramebufferWidth, m_GBuffer->m_FramebufferHeight, 0, 0, m_RenderWindowSize.x, m_RenderWindowSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		
		//6) Custom Forward Render Pass
		m_RenderTargetsCustom.push_back(nullptr);
		for (unsigned int targetIndex = 0; targetIndex < m_RenderTargetsCustom.size(); targetIndex++)
		{
			RenderTarget* renderTarget = m_RenderTargetsCustom[targetIndex];
			if (renderTarget)
			{
				glViewport(0, 0, renderTarget->m_FramebufferWidth, renderTarget->m_FramebufferHeight);
				glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->m_FramebufferID);
				if (renderTarget->m_HasDepthAndStencilAttachments)
				{
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				}
				else
				{
					glClear(GL_COLOR_BUFFER_BIT);
				}
				m_Camera->SetPerspectiveMatrix(glm::radians(60.0f), ((float)renderTarget->m_FramebufferWidth / (float)renderTarget->m_FramebufferHeight), 0.2f, 100.0f);
			}
			else
			{
				//Don't render to default framebuffer, but to custom target framebuffer which we will use for postprocessing.
				glViewport(0, 0, m_RenderWindowSize.x, m_RenderWindowSize.y);
				glBindFramebuffer(GL_FRAMEBUFFER, m_CustomRenderTarget->m_FramebufferID);
				m_Camera->SetPerspectiveMatrix(m_Camera->m_FieldOfView, m_RenderWindowSize.x / m_RenderWindowSize.y, 0.1f, 100.0f);
			}

			///Render custom commands here. (Things with custom material). By default, we will have 1 for the sky.
			std::vector<RenderCommand> renderCommands = m_RenderQueue->RetrieveCustomRenderCommands(renderTarget);

			//Iterate over all render commands and execute.
			m_GLStateCache->SetPolygonMode(m_WireframesEnabled ? GL_LINE : GL_FILL);
			for (unsigned int i = 0; i < renderCommands.size(); i++)
			{
				RenderCustomCommand(&renderCommands[i], nullptr);
			}
			m_GLStateCache->SetPolygonMode(GL_FILL);
		}

		//7) Alpha Material Pass
		//glViewport(0, 0, m_RenderWindowSize.x, m_RenderWindowSize.y);
		//glBindFramebuffer(GL_FRAMEBUFFER, m_CustomRenderTarget->m_FramebufferID);
		///Alpha Render Commands Here.

		//Render Light Mesh (as visual cue), if requested.
		for (auto iterator = m_PointLights.begin(); iterator != m_PointLights.end(); iterator++)
		{
			if ((*iterator)->m_RenderMesh)
			{
				m_MaterialLibrary->m_DebugLightMaterial->SetShaderVector3("lightColor", (*iterator)->m_LightColor * (*iterator)->m_LightIntensity * 0.25f);
			
				RenderCommand renderCommand;
				renderCommand.m_Material = m_MaterialLibrary->m_DebugLightMaterial;
				renderCommand.m_Mesh = m_DebugLightMesh;

				glm::mat4 lightModelMatrix = glm::mat4(1.0f); //Not the light space matrix. Just our model matrix for the light itself.
				lightModelMatrix = glm::translate(lightModelMatrix, (*iterator)->m_LightPosition);
				lightModelMatrix = glm::scale(lightModelMatrix, glm::vec3(0.25f));
				renderCommand.m_Transform = lightModelMatrix;

				RenderCustomCommand(&renderCommand, nullptr);
			}
		}

		//8) Pody-Processing Stage after all lighting calculations.

		//9) Render Debug Visuals
		glViewport(0, 0, m_RenderWindowSize.x, m_RenderWindowSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_CustomRenderTarget->m_FramebufferID);
		if (m_ShowDebugLightVolumes)
		{
			m_GLStateCache->SetPolygonMode(GL_LINE);
			m_GLStateCache->SetCulledFace(GL_FRONT);
			for (auto iterator = m_PointLights.begin(); iterator != m_PointLights.end(); iterator++)
			{
				m_MaterialLibrary->m_DebugLightMaterial->SetShaderVector3("lightColor", (*iterator)->m_LightColor);

				RenderCommand renderCommand;
				renderCommand.m_Material = m_MaterialLibrary->m_DebugLightMaterial;
				renderCommand.m_Mesh = m_DebugLightMesh;

				glm::mat4 lightDebugModelMatrix = glm::mat4(1.0f); //Not the light space matrix. Just our model matrix for the light debug mesh itself.
				lightDebugModelMatrix = glm::translate(lightDebugModelMatrix, (*iterator)->m_LightPosition);
				lightDebugModelMatrix = glm::scale(lightDebugModelMatrix, glm::vec3((*iterator)->m_LightRadius));
				renderCommand.m_Transform = lightDebugModelMatrix;

				RenderCustomCommand(&renderCommand, nullptr);
			}
			m_GLStateCache->SetPolygonMode(GL_FILL);
			m_GLStateCache->SetCulledFace(GL_BACK);
		}

		//10) Custom Post Processing Pass
		std::vector<RenderCommand> postProcessingCommands = m_RenderQueue->RetrievePostProcessingRenderCommands();
		for (unsigned int i = 0; i < postProcessingCommands.size(); i++)
		{
			//Ping Pong
			bool even = i % 2 == 0;
			Blit(even ? m_CustomRenderTarget->RetrieveColorAttachment(0) : m_PostProcessRenderTarget->RetrieveColorAttachment(0),
				even ? m_PostProcessRenderTarget : m_CustomRenderTarget, postProcessingCommands[i].m_Material);
		}

		//11) Finally, Blit everything to our framebuffer for rendering.
		BlitToMainFramebuffer(postProcessingCommands.size() % 2 == 0 ? m_CustomRenderTarget->RetrieveColorAttachment(0) : m_PostProcessRenderTarget->RetrieveColorAttachment(0));

		m_RenderQueue->ClearQueuedCommands();
		m_RenderTargetsCustom.clear();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::BlitToMainFramebuffer(Texture* sourceRenderTarget)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_MainRenderTarget->m_FramebufferID);
		glViewport(0, 0, m_RenderWindowSize.x, m_RenderWindowSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//Bind Input Texture Data
		sourceRenderTarget->BindTexture(0);

		m_GBuffer->RetrieveColorAttachment(3)->BindTexture(5);

		m_PostProcessor->m_PostProcessingShader->UseShader();
		m_PostProcessor->m_PostProcessingShader->SetUniformBool("SSAO", true);

		RenderMesh(m_NDCQuad);
	}

	void Renderer::Blit(Texture* textureSource, RenderTarget* targetDestination, Material* material, std::string textureUniformName)
	{
		//If a destination is given, bind to its framebuffer.
		if (targetDestination)
		{
			glViewport(0, 0, targetDestination->m_FramebufferWidth, targetDestination->m_FramebufferHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, targetDestination->m_FramebufferID);
			if (targetDestination->m_HasDepthAndStencilAttachments)
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			}
			else
			{
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}
		//Else, we bind to the default framebuffer.
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_MainRenderTarget->m_FramebufferID);
			glViewport(0, 0, m_RenderWindowSize.x, m_RenderWindowSize.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}
		//If no material was given, we use our default blit material.
		if (!material)
		{
			material = m_MaterialLibrary->m_DefaultBlitMaterial;
		}
		//If a source render target is given, we use its color buffer as input to the material shader.
		if (textureSource)
		{
			material->SetShaderTexture(textureUniformName, textureSource, 0);
		}
		//Render screen-space material to Quad which will be displayed in the destination's buffers.
		RenderCommand renderCommand;
		renderCommand.m_Material = material;
		renderCommand.m_Mesh = m_NDCQuad;
		RenderCustomCommand(&renderCommand, nullptr);
	}

	void Renderer::RenderCubemap(SceneEntity* sceneEntity, TextureCube* cubemapTarget, glm::vec3 position, unsigned int mipmappingLevel)
	{
		//We create a render queue just for this operation as to not conflict with our main command buffer. Rendering a cubemap in PBR is after all a chain of commands in itself.
		RenderQueue renderQueue(this);
		renderQueue.PushToRenderQueue(sceneEntity->m_Mesh, sceneEntity->m_Material, sceneEntity->RetrieveEntityTransform());

		std::vector<RenderCommand> renderCommands = renderQueue.RetrieveCustomRenderCommands(nullptr);
		RenderCubemap(renderCommands, cubemapTarget, position, mipmappingLevel);
	}

	void Renderer::RenderCubemap(std::vector<RenderCommand>& renderCommands, TextureCube* cubeTarget, glm::vec3 position, unsigned int mipmappingLevel)
	{
		//Define 6 camera directions/lookup vectors.
		Camera faceCameras[6] =
		{
			Camera(position, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			Camera(position, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			Camera(position, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			Camera(position, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			Camera(position, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			Camera(position, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		//Resize target dimensions based on the mipmap level we're rendering.
		float width = (float)cubeTarget->m_TextureCubeFaceWidth * std::pow(0.5f, mipmappingLevel);
		float height = (float)cubeTarget->m_TextureCubeFaceHeight * std::pow(0.5f, mipmappingLevel);

		glBindFramebuffer(GL_FRAMEBUFFER, m_CubemapFramebufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_CubemapDepthRenderbufferID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_CubemapDepthRenderbufferID);

		//Resize the relevant buffers.
		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, m_CubemapFramebufferID);

		for (unsigned int i = 0; i < 6; i++) //Inject the actual texture data (We only default initialized it).
		{
			Camera* camera = &faceCameras[i];
			camera->m_ProjectionMatrix = glm::perspective(glm::radians(90.0f), width / height, 0.1f, 100.0f);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeTarget->m_TextureCubeID, mipmappingLevel);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (unsigned int i = 0; i < renderCommands.size(); i++)
			{
				//Cubemap generation only works with custom materials.
				if (renderCommands[i].m_Material->m_MaterialType != Material_Custom)
				{
					CrescentError("Material used to generate cubemap is not a Custom Material.");
				}
				RenderCustomCommand(&renderCommands[i], camera);
			}
		}
	}

	//Renders from the light's point of view. 
	void Renderer::RenderShadowCastCommand(RenderCommand* renderCommand, const glm::mat4& lightSpaceProjectionMatrix, const glm::mat4& lightSpaceViewMatrix)
	{
		Shader* shadowShader = m_MaterialLibrary->m_DirectionalShadowShader;

		shadowShader->UseShader();
		shadowShader->SetUniformMat4("lightSpaceProjection", lightSpaceProjectionMatrix);
		shadowShader->SetUniformMat4("lightSpaceView", lightSpaceViewMatrix);
		shadowShader->SetUniformMat4("model", renderCommand->m_Transform);

		RenderMesh(renderCommand->m_Mesh);
	}

	void Renderer::RenderDeferredDirectionalLight(DirectionalLight* directionalLight)
	{
		//We also have to update the global uniform buffer for this.
		Shader* directionalShader = m_MaterialLibrary->m_DeferredDirectionalLightShader;

		directionalShader->UseShader();
		directionalShader->SetUniformVector3("cameraPosition", m_Camera->m_CameraPosition);
		directionalShader->SetUniformVector3("lightDirection", directionalLight->m_LightDirection);
		directionalShader->SetUniformVector3("lightColor", glm::normalize(directionalLight->m_LightColor) * directionalLight->m_LightIntensity);
		directionalShader->SetUniformBool("ShadowsEnabled", m_ShadowsEnabled);

		if (directionalLight->m_ShadowMapRenderTarget)
		{
			directionalShader->SetUniformMat4("lightShadowViewProjection", directionalLight->m_LightSpaceViewProjectionMatrix);
			directionalLight->m_ShadowMapRenderTarget->RetrieveDepthAndStencilAttachment()->BindTexture(3); //In our material library, we set the shadow map sampler to be in texture slot 3.
		}

		RenderMesh(m_NDCQuad);
	}

	void Renderer::RenderDeferredAmbientLight()
	{
		EnvironmentalPBR* skyCapture = m_PBR->RetrieveSkyCapture();

		//Do a full screen ambient pass.
		if (m_IBLAmbience)
		{
			skyCapture->m_IrradianceTextureCube->BindTextureCube(3);
			skyCapture->m_PrefilteredTextureCube->BindTextureCube(4);
			m_PBR->m_RenderTargetBRDFLUT->RetrieveColorAttachment(0)->BindTexture(5);
			m_PostProcessor->m_SSAOOutput->BindTexture(6);
			
			Shader* ambientShader = m_MaterialLibrary->m_DeferredAmbientLightShader;
			ambientShader->UseShader();
			ambientShader->SetUniformVector3("camPos", m_Camera->m_CameraPosition);
			ambientShader->SetUniformInteger("SSAO", true);
			RenderMesh(m_NDCQuad);
		}
	}

	void Renderer::RenderDeferredPointLight(PointLight* pointLight)
	{
		Shader* pointLightShader = m_MaterialLibrary->m_DeferredPointLightShader;

		pointLightShader->UseShader();
		pointLightShader->SetUniformVector3("cameraPosition", m_Camera->m_CameraPosition);
		pointLightShader->SetUniformVector3("lightPosition", pointLight->m_LightPosition);
		pointLightShader->SetUniformFloat("lightRadius", pointLight->m_LightRadius);
		pointLightShader->SetUniformVector3("lightColor", glm::normalize(pointLight->m_LightColor) * pointLight->m_LightIntensity);

		glm::mat4 pointLightModelMatrix = glm::mat4(1.0f);
		pointLightModelMatrix = glm::translate(pointLightModelMatrix, pointLight->m_LightPosition);
		pointLightModelMatrix = glm::scale(pointLightModelMatrix, glm::vec3(pointLight->m_LightRadius));

		pointLightShader->SetUniformMat4("projection", m_Camera->m_ProjectionMatrix);
		pointLightShader->SetUniformMat4("view", m_Camera->m_ViewMatrix);
		pointLightShader->SetUniformMat4("model", pointLightModelMatrix);

		RenderMesh(m_DeferredPointLightMesh);
	}

	void Renderer::RenderCustomCommand(RenderCommand* renderCommand, Camera* customRenderCamera, bool updateGLStates)
	{
		Mesh* mesh = renderCommand->m_Mesh;
		Material* material = renderCommand->m_Material;

		//Update global OpenGL states based on the material.
		if (updateGLStates)
		{
			m_GLStateCache->ToggleBlending(material->m_BlendingEnabled);
			if (material->m_BlendingEnabled)
			{
				m_GLStateCache->SetBlendingFunction(material->m_BlendSource, material->m_BlendDestination);
			}
			m_GLStateCache->ToggleDepthTesting(material->m_DepthTestEnabled);
			m_GLStateCache->SetDepthFunction(material->m_DepthTestFunction);

			m_GLStateCache->ToggleFaceCulling(material->m_FaceCullingEnabled);
			m_GLStateCache->SetCulledFace(material->m_CulledFace);
		}

		//Default uniforms that are always configured regardless of shader configuration. See these as a set of default shader variables that are always there.
		///To implement with Uniform Buffer Objects.
		material->RetrieveMaterialShader()->UseShader();
		material->RetrieveMaterialShader()->SetUniformMat4("projection", m_Camera->m_ProjectionMatrix);
		material->RetrieveMaterialShader()->SetUniformMat4("view", m_Camera->m_ViewMatrix);
		material->RetrieveMaterialShader()->SetUniformVector3("cameraPosition", m_Camera->m_CameraPosition);

		if (customRenderCamera) //If a custom camera is defined, we will update our shader uniforms with its information as needed.
		{
			material->RetrieveMaterialShader()->SetUniformMat4("projection", customRenderCamera->m_ProjectionMatrix);
			material->RetrieveMaterialShader()->SetUniformMat4("view", customRenderCamera->m_ViewMatrix);
			material->RetrieveMaterialShader()->SetUniformVector3("cameraPosition", customRenderCamera->m_CameraPosition);
		}

		//==============================================
		material->RetrieveMaterialShader()->SetUniformMat4("model", renderCommand->m_Transform);

		///Shadow Related Stuff. Create Shaders for relevant stuff in Material Library.
		material->RetrieveMaterialShader()->SetUniformBool("ShadowsEnabled", m_ShadowsEnabled); //If global shadows are enabled.
		if (m_ShadowsEnabled && material->m_MaterialType == Material_Custom && material->m_ShadowReceiving) //If the mesh in question should receive shadows...
		{
			for (int i = 0; i < m_DirectionalLights.size(); i++)
			{
				if (m_DirectionalLights[i]->m_ShadowMapRenderTarget != nullptr)
				{
					material->RetrieveMaterialShader()->SetUniformMat4("lightShadowViewProjection" + std::to_string(i + 1), m_DirectionalLights[i]->m_LightSpaceViewProjectionMatrix);
					m_DirectionalLights[i]->m_ShadowMapRenderTarget->RetrieveDepthAndStencilAttachment()->BindTexture(10 + i);
				}
			}
		}

		//Bind and set active uniform sampler/texture objects.
		auto* samplers = material->GetSamplerUniforms(); //Returns a map of a string (uniform name) and its corresponding uniform information.
		for (auto iterator = samplers->begin(); iterator != samplers->end(); iterator++)
		{
			if (iterator->second.m_UniformType == Shader_Type_SamplerCube)
			{
				iterator->second.m_TextureCube->BindTextureCube(iterator->second.m_TextureUnit);
			}
			else
			{
				iterator->second.m_Texture->BindTexture(iterator->second.m_TextureUnit);
			}
		}

		//Set uniform states of material.
		auto* uniforms = material->GetUniforms(); //Returns a map of a string (uniform name) and its corresponding uniform information.
		for (auto iterator = uniforms->begin(); iterator != uniforms->end(); iterator++)
		{
			switch (iterator->second.m_UniformType)
			{
			case Shader_Type_Boolean:
				material->RetrieveMaterialShader()->SetUniformBool(iterator->first, iterator->second.m_BoolValue);
				break;
			case Shader_Type_Integer:
				material->RetrieveMaterialShader()->SetUniformInteger(iterator->first, iterator->second.m_IntValue);
				break;
			case Shader_Type_Float:
				material->RetrieveMaterialShader()->SetUniformFloat(iterator->first, iterator->second.m_FloatValue);
				break;
			//case Shader_Type_Vec2:
				//material->RetrieveMaterialShader()->SetUniformVector2(it->first, it->second.Vec2);
				//break;
			case Shader_Type_Vector3:
				material->RetrieveMaterialShader()->SetUniformVector3(iterator->first, iterator->second.m_Vector3Value);
				break;
			//case Shader_Type_Vec4:
				//material->RetrieveMaterialShader()->SetVector(it->first, it->second.Vec4);
				//break;
			//case Shader_Type_Mat2:
				//material->RetrieveMaterialShader()->SetMatrix(it->first, it->second.Mat2);
				//break;
			//case Shader_Type_Mat3:
				//material->RetrieveMaterialShader()->SetMatrix(it->first, it->second.Mat3);
				//break;
			case Shader_Type_Matrix4:
				material->RetrieveMaterialShader()->SetUniformMat4(iterator->first, iterator->second.m_Mat4Value);
				break;
			default:
				CrescentError("You tried to set an unidentified uniform data type and value. Please check."); //Include which shader.
				break;
			}
		}

		RenderMesh(renderCommand->m_Mesh);
	}

	void Renderer::RenderMesh(Mesh* mesh)
	{
		glBindVertexArray(mesh->RetrieveVertexArrayID()); //Binding will automatically fill the vertex array with the attributes allocated during its time. 
		if (mesh->m_Indices.size() > 0)
		{
			glDrawElements(mesh->m_Topology == TriangleStrips ? GL_TRIANGLE_STRIP : GL_TRIANGLES, mesh->m_Indices.size(), GL_UNSIGNED_INT, 0);
		}
		else
		{
			glDrawArrays(mesh->m_Topology == TriangleStrips ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, mesh->m_Positions.size());
		}
	}

	void Renderer::SetRenderingWindowSize(int newWidth, int newHeight)
	{
		m_RenderWindowSize = glm::vec2(newWidth, newHeight);
		m_GBuffer->ResizeRenderTarget(newWidth, newHeight);
		m_CustomRenderTarget->ResizeRenderTarget(newWidth, newHeight);
		m_PostProcessRenderTarget->ResizeRenderTarget(newWidth, newHeight);
		m_MainRenderTarget->ResizeRenderTarget(newWidth, newHeight);

		m_PostProcessor->UpdatePostProcessingRenderTargetSizes(newWidth, newHeight);
	}

	RenderTarget* Renderer::RetrieveMainRenderTarget()
	{
		return m_MainRenderTarget;
	}

	RenderTarget* Renderer::RetrieveGBuffer()
	{
		return m_GBuffer;
	}

	RenderTarget* Renderer::RetrieveShadowRenderTarget(int index)
	{
		return m_ShadowRenderTargets[index];
	}

	RenderTarget* Renderer::RetrieveCustomRenderTarget()
	{
		return m_CustomRenderTarget;
	}

	void Renderer::SetSceneCamera(Camera* sceneCamera)
	{
		m_Camera = sceneCamera;
	}

	Camera* Renderer::RetrieveSceneCamera()
	{
		return m_Camera;
	}

	Material* Renderer::CreateMaterial(std::string shaderName)
	{
		return m_MaterialLibrary->CreateMaterial(shaderName);
	}

	void Renderer::AddLightSource(DirectionalLight* directionalLight)
	{
		m_DirectionalLights.push_back(directionalLight);
	}

	void Renderer::AddLightSource(PointLight* pointLight)
	{
		m_PointLights.push_back(pointLight);
	}

	EnvironmentalPBR* Renderer::RetrieveSkyCapture()
	{
		return m_PBR->RetrieveSkyCapture();
	}

	void Renderer::SetSkyCapture(EnvironmentalPBR* capturedEnvironment)
	{
		m_PBR->SetSkyCapture(capturedEnvironment);
	}

	void Renderer::UpdateGlobalUniformBufferObjects()
	{
		//glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUniformBufferID);
		//For now, we will update the global uniforms here. 
		//glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(m_Camera->m_ProjectionMatrix);
		//glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(m_Camera->GetViewMatrix()));
	}
}