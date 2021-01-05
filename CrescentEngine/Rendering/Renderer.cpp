#include "CrescentPCH.h";
#include "Renderer.h";
#include "RenderQueue.h"
#include "../Scene/SceneEntity.h"
#include "../Models/Model.h"
#include "Shader.h"

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
		m_RenderQueue = new RenderQueue(this);
	}

	void Renderer::PushToRenderQueue(SceneEntity* sceneEntity)
	{
		//Update transform before pushing node to render command buffer.
		sceneEntity->UpdateEntityTransform(true);

		m_RenderQueue->PushToRenderQueue(sceneEntity->m_Mesh, sceneEntity->m_Material, sceneEntity->RetrieveEntityTransform());
	}

	//Attach shader to material.
	void Renderer::RenderAllQueueItems(Shader* temporaryShader)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Rendering Commands
		std::vector<RenderCommand> forwardRenderCommands = m_RenderQueue->RetrieveForwardRenderingCommands();
		
		for (int i = 0; i < forwardRenderCommands.size(); i++)
		{
			temporaryShader->UseShader();
			temporaryShader->SetUniformMat4("model", forwardRenderCommands[i].m_Transform);
			forwardRenderCommands[i].m_Mesh->DrawStaticModel(*temporaryShader, false, 0);
		}

		m_RenderQueue->ClearQueuedCommands();
	}


	//======================================================================================


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

	void Renderer::ClearBuffers()
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}