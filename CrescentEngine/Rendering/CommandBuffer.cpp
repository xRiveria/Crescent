#include "CrescentPCH.h"
#include "CommandBuffer.h"
#include "Renderer.h"
#include "../Utilities/Camera.h"

namespace Crescent
{
	CommandBuffer::CommandBuffer(Renderer* renderer)
	{
		m_Renderer = renderer;
	}

	CommandBuffer::~CommandBuffer()
	{
		ClearCommandBuffer();
	}

	void CommandBuffer::PushToCommandBuffer(Mesh* mesh, Material* material, glm::mat4 transform, glm::mat4 previousTransform, glm::vec3 boundingBoxMinimum, glm::vec3 boundingBoxMaximum, RenderTarget* renderTarget)
	{
		RenderCommand renderCommand = {};
		renderCommand.m_Mesh = mesh;
		renderCommand.m_Material = material;
		renderCommand.m_Transform = transform;
		renderCommand.m_PreviousTransform = previousTransform;
		renderCommand.m_BoundingBoxMinimum = boundingBoxMinimum;
		renderCommand.m_BoundingBoxMaximum = boundingBoxMaximum;

		//If material requires alpha support, add it to alpha render commands for later rendering.
		if (material->m_BlendingEnabled)
		{
			material->m_MaterialType = Material_Custom;
			m_AlphaRenderCommands.push_back(renderCommand);
		}
		else
		{
			//Check the type of the material and process differently when necessary.
			if (material->m_MaterialType == Material_Default)
			{
				m_DeferredRenderCommands.push_back(renderCommand);
			}
			else if (material->m_MaterialType == Material_Custom)
			{
				//Check if this render target has been pushed before. If so, add to the vector. Else, we create a new vector with this render target.
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
			else if (material->m_MaterialType == Material_PostProcess)
			{
				m_PostProcessingRenderCommands.push_back(renderCommand);
			}
		}
	}

	void CommandBuffer::ClearCommandBuffer()
	{
		m_DeferredRenderCommands.clear();
		m_CustomRenderCommands.clear();
		m_PostProcessingRenderCommands.clear();
		m_AlphaRenderCommands.clear();
	}

	//===============================================================================================
	//Custom per-element sort compare function used by the CommandBuffer::Sort() function.
	bool RenderSortDeferred(const RenderCommand& a, const RenderCommand& b)
	{
		return a.m_Material->RetrieveMaterialShader()->RetrieveShaderID() < b.m_Material->RetrieveMaterialShader()->RetrieveShaderID();
	}
	
	//Sort render state.
	bool RenderSortCustom(const RenderCommand& a, const RenderCommand& b)
	{
		/*
			We want strict weak ordering, which states that if 2 objects X and Y are equivalent, then both F(X,Y) and F(Y, X) should be false. Thus, when comparing the object to itself,
			the comparison should always be false. We also want to do multiple sort comparisons in a single pass, so we encapsulate all relevant properties inside a N-Tuple with N being equal
			to the number of sort queries we want to do. The tuple < comparison operator compares its left-most element and then works along the next elements of the tuple until an outcome is clear.
		*/
		return std::make_tuple(a.m_Material->m_BlendingEnabled, a.m_Material->RetrieveMaterialShader()->RetrieveShaderID()) <
			   std::make_tuple(b.m_Material->m_BlendingEnabled, b.m_Material->RetrieveMaterialShader()->RetrieveShaderID());
	}

	void CommandBuffer::SortCommandBuffer()
	{
		std::sort(m_DeferredRenderCommands.begin(), m_DeferredRenderCommands.end(), RenderSortDeferred);
		for (auto iterator = m_CustomRenderCommands.begin(); iterator != m_CustomRenderCommands.end(); iterator++)
		{
			std::sort(iterator->second.begin(), iterator->second.end(), RenderSortCustom);
		}
	}

	//All culling related checks below exist for optimization to check if objects are within the camera's view frustrum before rendering. In the case of custom render commands, we only
	//want to cull if it is indeed enabled and when the render target is null/on the main render target. 
	std::vector<RenderCommand> CommandBuffer::GetDeferredRenderCommand(bool cullingEnabled)
	{
		if (cullingEnabled)
		{
			std::vector<RenderCommand> renderCommands;
			for (auto iterator = m_DeferredRenderCommands.begin(); iterator != m_DeferredRenderCommands.end(); ++iterator)
			{
				RenderCommand renderCommand = *iterator;
				if (m_Renderer->RetrieveCamera()->m_CameraFrustrum.Intersect(renderCommand.m_BoundingBoxMinimum, renderCommand.m_BoundingBoxMaximum))
				{
					renderCommands.push_back(renderCommand);
				}
			}
			return renderCommands;
		}
		else
		{
			return m_DeferredRenderCommands;
		}
	}

	std::vector<RenderCommand> CommandBuffer::GetAlphaRenderCommands(bool cullingEnabled)
	{
		if (cullingEnabled)
		{
			std::vector<RenderCommand> renderCommands;
			for (auto iterator = m_AlphaRenderCommands.begin(); iterator != m_AlphaRenderCommands.end(); ++iterator)
			{
				RenderCommand renderCommand = *iterator;
				if (m_Renderer->RetrieveCamera()->m_CameraFrustrum.Intersect(renderCommand.m_BoundingBoxMinimum, renderCommand.m_BoundingBoxMaximum))
				{
					renderCommands.push_back(renderCommand);
				}
			}
			return renderCommands;
		}
		else
		{
			return m_AlphaRenderCommands;
		}
	}

	std::vector<RenderCommand> CommandBuffer::GetCustomRenderCommands(RenderTarget* renderTarget, bool cullingEnabled)
	{
		//Only cull when on main/null render target.
		if (renderTarget == nullptr && cullingEnabled)
		{
			std::vector<RenderCommand> renderCommands;
			for (auto iterator = m_CustomRenderCommands[renderTarget].begin(); iterator != m_CustomRenderCommands[renderTarget].end(); ++iterator)
			{
				RenderCommand command = *iterator;
				if (m_Renderer->RetrieveCamera()->m_CameraFrustrum.Intersect(command.m_BoundingBoxMinimum, command.m_BoundingBoxMaximum))
				{
					renderCommands.push_back(command);
				}
			}
			return renderCommands;
		}
		else
		{
			return m_CustomRenderCommands[renderTarget];
		}
	}

	std::vector<RenderCommand> CommandBuffer::GetPostProcessingRenderCommands()
	{
		return m_PostProcessingRenderCommands;
	}

	std::vector<RenderCommand> CommandBuffer::GetShadowCastingRenderCommands()
	{
		std::vector<RenderCommand> renderCommands;
		for (auto iterator = m_DeferredRenderCommands.begin(); iterator != m_DeferredRenderCommands.end(); ++iterator)
		{
			if (iterator->m_Material->m_ShadowCasting)
			{
				renderCommands.push_back(*iterator);
			}
		}
		
		for (auto iterator = m_CustomRenderCommands[nullptr].begin(); iterator != m_CustomRenderCommands[nullptr].end(); ++iterator) //Nullptr for RenderTarget.
		{
			if (iterator->m_Material->m_ShadowCasting)
			{
				renderCommands.push_back(*iterator);
			}
		}

		return renderCommands;
	}
}