#pragma once
#include "RenderCommand.h"
#include "Renderer.h"
#include "../Models/Mesh.h"
#include "Material.h"
#include "RenderTarget.h"

namespace Crescent
{
	/*
	Render command buffer, managing all per-frame render/draw calls and converts them to a more efficient render-friendly format for the renderer to execute.
	*/
	class CommandBuffer
	{
	public:






	private:
		Renderer* m_Renderer;
		std::vector<RenderCommand> m_DeferredRenderCommands;
		std::vector<RenderCommand> m_AlphaRenderCommands;
		std::vector<RenderCommand> m_PostProcessingRenderCommands;
		std::map<RenderTarget*, std::vector<RenderCommand>> m_CustomRenderCommands;
	};
}