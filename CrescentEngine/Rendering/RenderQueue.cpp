#include "CrescentPCH.h"
#include "RenderQueue.h"

namespace CrescentEngine
{
	void RenderQueue::SubmitToRenderQueue(const PrimitiveShape& primitiveShape)
	{
		PrimitiveShape selectedPrimitive(primitiveShape);
		m_Primitives.push_back(selectedPrimitive);
	}

	void RenderQueue::RenderAllQueueItems(Shader& shader)
	{
		for(Primitive& primitive : m_Primitives)
		{
			primitive.DrawPrimitive(shader);
		}
	}

	void RenderQueue::RenderAllQueueEditorSettings()
	{
		for (Primitive& primitive : m_Primitives)
		{
			primitive.DrawEditorSettings();
		}
	}
}