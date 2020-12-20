#pragma once
#include <deque>
#include "Primitive.h"

namespace CrescentEngine
{
	class RenderQueue
	{
	public:
		RenderQueue() {}
		//Submission into Queue
		void SubmitToRenderQueue(const PrimitiveShape& primitiveShape);

		//Rendering
		void RenderAllQueueItems(LearnShader& shader);
		void RenderAllQueueEditorSettings();

	private:
		std::deque<Primitive> m_Primitives;
	};
}
