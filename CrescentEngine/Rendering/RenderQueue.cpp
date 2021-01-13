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

	void RenderQueue::PushToRenderQueue(Mesh* mesh, Material* material, glm::mat4 transform, RenderTarget* renderTarget)
	{
		RenderCommand renderCommand = {};

		renderCommand.m_Mesh = mesh;
		renderCommand.m_Material = material;
		renderCommand.m_Transform = transform;

		//Here, we will have different queue types for different rendering styles. We can filter with material types.
		if (material->m_BlendingEnabled)
		{
			//Add to blending command queue.
		}
		else
		{
			//We check the type of material we have and process differently where necessary.
			if (material->m_MaterialType == Material_Default)
			{
				m_DeferredRenderingCommands.push_back(renderCommand);
			}
			else if (material->m_MaterialType == Material_Custom)
			{
				//Check if this render target has been pushed before. If so, we add to its vector.
				//Otherwise, we create a new vector with this render target. 
				if (m_CustomRenderCommands.find(renderTarget) != m_CustomRenderCommands.end())
				{
					m_CustomRenderCommands[renderTarget].push_back(renderCommand);
				}
				else
				{
					m_CustomRenderCommands[renderTarget] = std::vector<RenderCommand>();
					m_CustomRenderCommands[renderTarget].push_back(renderCommand);
				}
			}
			//One more check if its a post-processing material. 
		}
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

	std::vector<RenderCommand> RenderQueue::RetrieveCustomRenderCommands(RenderTarget* renderTarget, bool cullingEnabled)
	{
		//Only do culling when on our main/null render target. Culling code here.

		return m_CustomRenderCommands[renderTarget]; //Return render commands belonging to the passed in render target.
	}

	std::vector<RenderCommand> RenderQueue::RetrievePostProcessingRenderCommands()
	{
		return m_PostProcessingRenderCommands;
	}

	void RenderQueue::ClearQueuedCommands()
	{
		m_DeferredRenderingCommands.clear();
		m_CustomRenderCommands.clear();
	}
}







