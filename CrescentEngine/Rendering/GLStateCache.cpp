#include "CrescentPCH.h"
#include "GLStateCache.h"

namespace Crescent
{
	GLStateCache::GLStateCache()
	{

	}

	GLStateCache::~GLStateCache()
	{

	}

	void GLStateCache::ToggleDepthTesting(bool depthTestingEnabled)
	{
		if (m_DepthTestEnabled != depthTestingEnabled)
		{
			m_DepthTestEnabled = depthTestingEnabled;
			depthTestingEnabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		}
	}

	void GLStateCache::SetDepthFunction(GLenum depthTestFunction)
	{
		if (m_DepthTestFunction != depthTestFunction)
		{
			m_DepthTestFunction = depthTestFunction;
			glDepthFunc(depthTestFunction);
		}
	}

	void GLStateCache::ToggleBlending(bool blendingEnabled)
	{
		if (m_BlendingEnabled != blendingEnabled)
		{
			m_BlendingEnabled = blendingEnabled;
			blendingEnabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		}
	}

	void GLStateCache::SetBlendingFunction(GLenum source, GLenum destination)
	{
		if (m_BlendSource != source || m_BlendDestination != destination)
		{
			m_BlendSource = source;
			m_BlendDestination = destination;
			glBlendFunc(source, destination);
		}
	}

	void GLStateCache::ToggleFaceCulling(bool faceCullingEnabled)
	{
		if (m_FaceCullingEnabled != faceCullingEnabled)
		{
			m_FaceCullingEnabled = faceCullingEnabled;
			faceCullingEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		}
	}

	void GLStateCache::SetCulledFace(GLenum culledFace)
	{
		if (m_CulledFace != culledFace)
		{
			m_CulledFace = culledFace;
			glCullFace(culledFace);
		}
	}

	void GLStateCache::SetPolygonMode(GLenum polygonMode)
	{
		if (m_PolygonMode != polygonMode)
		{
			m_PolygonMode = polygonMode;
			glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
		}
	}
}