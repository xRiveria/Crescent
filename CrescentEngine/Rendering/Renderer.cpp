#include "CrescentPCH.h";
#include "Renderer.h";
#include "RenderQueue.h"
#include "GLStateCache.h"
#include "MaterialLibrary.h"
#include "Framebuffer.h" //To be abstracted.
#include "../Utilities/Camera.h"
#include "../Scene/SceneEntity.h"
#include "../Models/Model.h"
#include "../Shading/Shader.h"
#include "../Shading/Material.h"

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

		m_RenderQueue = new RenderQueue(this);
		m_GLStateCache = new GLStateCache();
		m_MaterialLibrary = new MaterialLibrary();
		m_Framebuffer = new Framebuffer();

		m_RenderWindowSize = glm::vec2(renderWindowWidth, renderWindowHeight);
		m_Framebuffer->InitializeFramebuffer(renderWindowWidth, renderWindowHeight);
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

		m_GLStateCache->ToggleDepthTesting(true);
		m_GLStateCache->ToggleBlending(true);
		m_GLStateCache->ToggleFaceCulling(false);

		//Rendering Commands
		std::vector<RenderCommand> forwardRenderCommands = m_RenderQueue->RetrieveForwardRenderingCommands();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_Framebuffer->RetrieveColorAttachment());
		m_Framebuffer->BindFramebuffer();
		glViewport(0, 0, m_RenderWindowSize.x, m_RenderWindowSize.y);

		for (int i = 0; i < forwardRenderCommands.size(); i++)
		{
			RenderForwardPassCommand(&forwardRenderCommands[i], nullptr, false);
		}

		m_RenderQueue->ClearQueuedCommands();

		m_Framebuffer->UnbindFramebuffer();
	}

	void Renderer::SetRenderingWindowSize(const int& newWidth, const int& newHeight)
	{
		m_RenderWindowSize = glm::vec2(newWidth, newHeight);
		m_Framebuffer->ResizeFramebuffer(newWidth, newHeight);
	}

	void Renderer::RenderForwardPassCommand(RenderCommand* renderCommand, Camera* customRenderCamera, bool updateGLStates)
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

		if (customRenderCamera) //If a custom camera is defined, we will update our shader uniforms with its information as needed.
		{

		}

		//==============================================
		///To replace with a UBO!
		//For now, we will update the global uniforms here. 
		material->RetrieveMaterialShader()->SetUniformMat4("projection", m_Camera->m_ProjectionMatrix);
		material->RetrieveMaterialShader()->SetUniformMat4("view", m_Camera->GetViewMatrix());
		//material->RetrieveMaterialShader()->SetUniformVector3("cameraPosition", m_Camera->m_CameraPosition);
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
			glDrawElements(GL_TRIANGLES, mesh->m_Indices.size(), GL_UNSIGNED_INT, 0);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, mesh->m_Positions.size());
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
}