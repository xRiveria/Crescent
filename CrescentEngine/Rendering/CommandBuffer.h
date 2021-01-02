#pragma once
#include "RenderCommand.h"
#include "Renderer.h"

namespace Crescent
{
	/*
	Render command buffer, managing all per-frame render/draw calls and converts them to a more efficient render-friendly format for the renderer to execute.
	*/
	class CommandBuffer
	{
	public:
		CommandBuffer(Renderer* renderer);
		~CommandBuffer();

		//Pushes render state relevant to a single render call to the command buffer.
		void PushToCommandBuffer(Mesh* mesh, Material* material, glm::mat4 transform = glm::mat4(1.0f), glm::mat4 previousTransform = glm::mat4(1.0f), glm::vec3 boundingBoxMinimum = glm::vec3(-99999.0f), glm::vec3 boundingBoxMaximum = glm::vec3(99999.0f), RenderTarget* renderTarget = nullptr);
		//Clears the command buffer, usually done after issuing all stored render commands. 
		void ClearCommandBuffer();
		//Sorts the command buffer. First by shaders, then by texture binds.
		void SortCommandBuffer();

		//Returns the list of render commands. For minimizing state changes, it is advised to first call Sort() before retrieving and issuing the render commands.
		std::vector<RenderCommand> GetDeferredRenderCommand(bool cullingEnabled = false);
		//Returns the list of render commands of both deferred and forward pushes that require alpha blending; which has to be rendered last.
		std::vector<RenderCommand> GetAlphaRenderCommands(bool cullingEnabled = false);
		//Returns the list of custom render commands per render target.
		std::vector<RenderCommand> GetCustomRenderCommands(RenderTarget* renderTarget, bool cullingEnabled = false);
		//Returns the list of post-processing render commands.
		std::vector<RenderCommand> GetPostProcessingRenderCommands();
		//Returns the list of all render commands with mesh shadow casting.
		std::vector<RenderCommand> GetShadowCastingRenderCommands();

	private:
		std::vector<RenderCommand> m_DeferredRenderCommands;
		std::vector<RenderCommand> m_AlphaRenderCommands;
		std::vector<RenderCommand> m_PostProcessingRenderCommands;
		std::map<RenderTarget*, std::vector<RenderCommand>> m_CustomRenderCommands;

		Renderer* m_Renderer;
	};
}