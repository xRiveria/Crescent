#pragma once
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

	void GLStateCache::ToggleDepthTesting(bool value)
	{
		if (m_DepthTestEnabled != value)
		{
			m_DepthTestEnabled = value;
			value ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		}
	}

	void GLStateCache::SetDepthTestingFunction(GLenum depthTestingFunction)
	{
		if (m_DepthTestingFunction != depthTestingFunction)
		{
			m_DepthTestingFunction = depthTestingFunction;
			glDepthFunc(depthTestingFunction);
		}
	}

	void GLStateCache::ToggleBlending(bool value)
	{
		if (m_BlendingEnabled != value)
		{
			m_BlendingEnabled = value;
			value ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
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

	void GLStateCache::ToggleFaceCulling(bool value)
	{
		if (m_FaceCullingEnabled != value)
		{
			m_FaceCullingEnabled = value;
			value ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		}
	}

	void GLStateCache::SetCullFace(GLenum face)
	{
		if (m_CulledFace != face)
		{
			m_CulledFace = face;
			glCullFace(face);
		}
	}

	void GLStateCache::SetPolygonMode(GLenum mode)
	{
		if (m_PolygonMode != mode)
		{
			m_PolygonMode = mode;
			glPolygonMode(GL_FRONT_AND_BACK, mode);
		}
	}

	void GLStateCache::SwitchShader(unsigned int ID)
	{
		if (m_ActiveShaderID != ID)
		{
			glUseProgram(ID);
		}
	}
}