#include "CrescentPCH.h"
#include "RenderQueue.h"
#include "RenderCommand.h"
#include "../Shading/Material.h"

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
		m_DeferredRenderingCommands.push_back(renderCommand);
	}

	std::vector<RenderCommand> RenderQueue::RetrieveDeferredRenderingCommands()
	{
		//Add cull check here.
		return m_DeferredRenderingCommands;
	}

	std::vector<RenderCommand> RenderQueue::RetrieveShadowCastingRenderCommands()
	{
		std::vector<RenderCommand> renderCommands;
		for (auto iterator = m_DeferredRenderingCommands.begin(); iterator != m_DeferredRenderingCommands.end(); iterator++)
		{
			if (iterator->m_Material->m_ShadowCasting)
			{
				renderCommands.push_back(*iterator);
			}
		}

		for (auto iterator = m_CustomRenderCommands[nullptr].begin(); iterator != m_CustomRenderCommands[nullptr].end(); iterator++)
		{
			if (iterator->m_Material->m_ShadowCasting)
			{
				renderCommands.push_back(*iterator);
			}
		}

		return renderCommands;
	}

	void RenderQueue::ClearQueuedCommands()
	{
		m_DeferredRenderingCommands.clear();
	}
}







