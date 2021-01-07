#include "CrescentPCH.h"
#include "RenderQueue.h"
#include "RenderCommand.h"

namespace Crescent
{
	RenderQueue::RenderQueue(Renderer* renderer)
	{
		m_Renderer = renderer;
	}

	void RenderQueue::PushToRenderQueue(Mesh* mesh, Material* material, glm::mat4 transform)
	{
		RenderCommand renderCommand = {};

		renderCommand.m_Mesh = mesh;
		renderCommand.m_Material = material;
		renderCommand.m_Transform = transform;

		//Here, we will have different queue types for different rendering styles. We can filter with material types. For now, we only have Forward Rendering.
		m_ForwardRenderingCommands.push_back(renderCommand);
	}

	std::vector<RenderCommand> RenderQueue::RetrieveForwardRenderingCommands()
	{
		return m_ForwardRenderingCommands;
	}

	void RenderQueue::ClearQueuedCommands()
	{
		m_ForwardRenderingCommands.clear();
	}
}







