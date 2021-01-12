#pragma once
#include <GL/glew.h>

namespace Crescent
{
	/*
		This state cache stores the latest relevant OpenGL state and only through the use of public setters can OpenGL's actual global state be altered.
		Switching OpenGL states (such as shaders, depth tests and blend states) can be quite expensive. By propagating every change through this cache,
		we can prevent unneccessary state changes.
	*/

	class GLStateCache
	{
	public:
		GLStateCache();
		~GLStateCache();

		//Update OpenGL state if requested state is different from current OpenGL state.
		void ToggleDepthTesting(bool depthTestingEnabled);
		void SetDepthFunction(GLenum depthTestFunction);

		void ToggleBlending(bool blendingEnabled);
		void SetBlendingFunction(GLenum source, GLenum destination);

		void ToggleFaceCulling(bool faceCullingEnabled);
		void SetCulledFace(GLenum culledFace);

		void SetPolygonMode(GLenum polygonMode);

	private:
		//Toggles
		bool m_DepthTestEnabled = false;
		bool m_BlendingEnabled = false; 
		bool m_FaceCullingEnabled = false;

		//States
		GLenum m_DepthTestFunction = 0;
		GLenum m_BlendSource = 0;
		GLenum m_BlendDestination = 0;
		GLenum m_CulledFace = 0;
		GLenum m_PolygonMode = 0;
	};
};
