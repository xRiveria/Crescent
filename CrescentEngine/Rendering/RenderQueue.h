#pragma once
#include "RenderCommand.h"
#include <deque>
#include <map>

namespace Crescent
{
	class Renderer;
	class Mesh;
	class Material;
	class RenderTarget;

	class RenderQueue
	{
	public:
		RenderQueue(Renderer* renderer);
		~RenderQueue();

		void PushToRenderQueue(Mesh* model, Material* material, glm::mat4 transform, RenderTarget* renderTarget = nullptr);
		std::vector<RenderCommand> RetrieveDeferredRenderingCommands();

		//Returns the list of all render commands with mesh shadow casting.
		std::vector<RenderCommand> RetrieveShadowCastingRenderCommands();

		std::vector<RenderCommand> RetrievePostProcessingRenderCommands();
		//Returns a list of custom render commands for a specific render target.
		std::vector<RenderCommand> RetrieveCustomRenderCommands(RenderTarget* renderTarget, bool cullingEnabled = false);

		void ClearQueuedCommands();

	private:
		std::vector<RenderCommand> m_DeferredRenderingCommands;
		std::vector<RenderCommand> m_PostProcessingRenderCommands;
		std::map<RenderTarget*, std::vector<RenderCommand>> m_CustomRenderCommands;
		Renderer* m_Renderer;
	};
}
