#pragma once
#include <deque>
#include "Primitive.h"

namespace Crescent
{
	class RenderQueue
	{
	public:
		RenderQueue() {}
		//Submission into Queue
		void SubmitToRenderQueue(const PrimitiveShape& primitiveShape);

		//Rendering
		void RenderAllQueueItems(Shader& shader);
		void RenderAllQueueEditorSettings();

	private:
		std::deque<Primitive> m_Primitives;
	};
}
