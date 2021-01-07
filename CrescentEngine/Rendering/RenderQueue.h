#pragma once
#include <deque>
#include "RenderCommand.h"

namespace Crescent
{
	class Renderer;
	class Mesh;
	class Material;

	class RenderQueue
	{
	public:
		RenderQueue() {}
		RenderQueue(Renderer* renderer);

		void PushToRenderQueue(Mesh* model, Material* material, glm::mat4 transform);
		std::vector<RenderCommand> RetrieveForwardRenderingCommands();
		void ClearQueuedCommands();

	private:
		std::vector<RenderCommand> m_ForwardRenderingCommands;
		Renderer* m_Renderer;
	};
}
