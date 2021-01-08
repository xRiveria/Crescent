#include "CrescentPCH.h";
#include "Renderer.h";
#include "RenderQueue.h"
#include "GLStateCache.h"
#include "MaterialLibrary.h"
#include "../Utilities/Camera.h"
#include "../Scene/SceneEntity.h"
#include "../Models/Model.h"
#include "../Shading/Shader.h"
#include "../Shading/Material.h"
#include "../Lighting/DirectionalLight.h"
#include <glm/gtc/type_ptr.hpp>

//As of now, our renderer only supports Forward Pass Rendering.

namespace Crescent
{
	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::InitializeRenderer(const int& renderWindowWidth, const int& renderWindowHeight)
	{
		if (glewInit() != GLEW_OK)
		{
			CrescentError("Failed to initialize GLEW.")
		}
		CrescentInfo("Successfully initialized GLEW.");

		m_DeviceRendererInformation = (char*)glGetString(GL_RENDERER);
		m_DeviceVendorInformation = (char*)glGetString(GL_VENDOR);
		m_DeviceVersionInformation = (char*)glGetString(GL_VERSION);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepth(1.0f);

		m_RenderQueue = new RenderQueue(this);
		m_GLStateCache = new GLStateCache();
		m_GBuffer = new RenderTarget(1, 1, GL_HALF_FLOAT, 4, true);
		m_MaterialLibrary = new MaterialLibrary(m_GBuffer);

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

		// configure default OpenGL state
		m_GLStateCache->ToggleDepthTesting(true);
		m_GLStateCache->ToggleFaceCulling(true);

		//Global Uniform Buffer Object
		//glGenBuffers(1, &m_GlobalUniformBufferID);
		//glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUniformBufferID);
		//glBufferData(GL_UNIFORM_BUFFER, 720, nullptr, GL_STREAM_DRAW);
		//glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_GlobalUniformBufferID);

		m_RenderWindowSize = glm::vec2(renderWindowWidth, renderWindowHeight);
	}

	void Renderer::PushToRenderQueue(SceneEntity* sceneEntity)
	{
		//Update transform before pushing node to render command buffer.
		sceneEntity->UpdateEntityTransform(true);

		m_RenderQueue->PushToRenderQueue(sceneEntity->m_Mesh, sceneEntity->m_Material, sceneEntity->RetrieveEntityTransform());
	}

	//Attach shader to material.
	void Renderer::RenderAllQueueItems()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Update Global Uniform Buffer Object
		UpdateGlobalUniformBufferObjects();

		//Set default OpenGL state.
		m_GLStateCache->ToggleBlending(true);
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

		for (int i = 0; i < deferredRenderCommands.size(); i++)
		{
			RenderCustomCommand(&deferredRenderCommands[i], nullptr, false);
		}

		//Disable for next pass (shadow map generation).
		attachments[1] = GL_NONE;
		attachments[2] = GL_NONE;
		attachments[3] = GL_NONE;
		glDrawBuffers(4, attachments);

		//2) Render All Shadow Casters to Light Shadow Buffers
		bool m_ShadowsEnbled = true; ///Make Global
		if (m_ShadowsEnbled)
		{
			m_GLStateCache->SetCulledFace(GL_FRONT);
			std::vector<RenderCommand> shadowRenderCommands = m_RenderQueue->RetrieveShadowCastingRenderCommands();
			m_ShadowViewProjectionMatrixes.clear();

			unsigned int shadowRenderTargetIndex = 0;
			for (int i = 0; i < m_DirectionalLights.size(); i++)
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

					for (int j = 0; j < shadowRenderCommands.size(); j++)
					{
						std::cout << "Found render command for " << shadowRenderCommands.size() << "Objects!";
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

		//4) Render deferred shader for each light (full quad for directional, spheres for point lights).



		m_RenderQueue->ClearQueuedCommands();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::RenderShadowCastCommand(RenderCommand* renderCommand, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
	{
		Shader* shadowShader = m_MaterialLibrary->m_DirectionalShadowShader;

		shadowShader->SetUniformMat4("projection", projectionMatrix);
		shadowShader->SetUniformMat4("view", viewMatrix);
		shadowShader->SetUniformMat4("model", renderCommand->m_Transform);

		RenderMesh(renderCommand->m_Mesh);
	}

	void Renderer::RenderDirectionalLight(DirectionalLight* directionalLight)
	{
		//We also have to update the global uniform buffer for this.

	}

	void Renderer::UpdateGlobalUniformBufferObjects()
	{
		//glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUniformBufferID);
		//For now, we will update the global uniforms here. 
		//glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(m_Camera->m_ProjectionMatrix);
		//glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(m_Camera->GetViewMatrix()));
	}

	void Renderer::SetRenderingWindowSize(const int& newWidth, const int& newHeight)
	{
		m_RenderWindowSize = glm::vec2((float)newWidth, (float)newHeight);
		m_GBuffer->ResizeRenderTarget(newWidth, newHeight);
	}

	RenderTarget* Renderer::RetrieveCurrentRenderTarget()
	{
		return m_CurrentCustomRenderTarget;
	}

	RenderTarget* Renderer::RetrieveGBuffer()
	{
		return m_GBuffer;
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
		//This works together with our Uniform Buffer Object. 
		material->RetrieveMaterialShader()->UseShader();

		material->RetrieveMaterialShader()->SetUniformMat4("projection", m_Camera->m_ProjectionMatrix);
		material->RetrieveMaterialShader()->SetUniformMat4("view", m_Camera->GetViewMatrix());
		//material->RetrieveMaterialShader()->SetUniformVector3("cameraPosition", m_Camera->m_CameraPosition);

		if (customRenderCamera) //If a custom camera is defined, we will update our shader uniforms with its information as needed.
		{

		}

		//==============================================
		material->RetrieveMaterialShader()->SetUniformMat4("model", renderCommand->m_Transform);

		///Shadow Related Stuff.

		//Bind and set active uniform sampler/texture objects.
		auto* samplers = material->GetSamplerUniforms(); //Returns a map of a string (uniform name) and its corresponding uniform information.
		for (auto iterator = samplers->begin(); iterator != samplers->end(); iterator++)
		{
			iterator->second.m_Texture->BindTexture(iterator->second.m_TextureUnit);
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

	void Renderer::SetCurrentRenderTarget(RenderTarget* renderTarget, GLenum framebufferTarget)
	{
		m_CurrentCustomRenderTarget = renderTarget;
		if (renderTarget != nullptr)
		{
			//Add to our cached target list. 
		}
	}
}