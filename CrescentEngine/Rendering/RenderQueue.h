#pragma once
#include <deque>
#include "Primitive.h"
#include "RenderCommand.h"

namespace Crescent
{
	class Renderer;
	class Model;
	class Material;

	class RenderQueue
	{
	public:
		RenderQueue() {}
		RenderQueue(Renderer* renderer);

		//Submission into Queue
		void SubmitToRenderQueue(const PrimitiveShape& primitiveShape);

		//Rendering
		void RenderAllQueueItems(Shader& shader);

		//New
		void PushToRenderQueue(Model* model, Material* material, glm::mat4 transform);
		std::vector<RenderCommand> RetrieveForwardRenderingCommands();
		void ClearQueuedCommands();

	private:
		std::vector<RenderCommand> m_ForwardRenderingCommands;
		Renderer* m_Renderer;

		std::deque<Primitive> m_Primitives;
	};
}
