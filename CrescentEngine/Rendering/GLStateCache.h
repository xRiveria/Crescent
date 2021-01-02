#pragma once
#include <GL/glew.h>

namespace Crescent
{
	/*
		GLStateCache stores the latest state of the relevant OpenGL state and through the use of public setters, it only updates the actual OpenGL state when its value is different
		from before. Switching GL states such as depth tests, blend states etc can be expensive. By propagating every state change through the use of GLStateCache, we prevent unnecessary
		state changes.
	*/

	class GLStateCache
	{
	public:
		GLStateCache();
		~GLStateCache();

		//Update state if requested state is different from the currently set state.
		void ToggleDepthTesting(bool value);
		void SetDepthTestingFunction(GLenum depthTestingFunction);

		void ToggleBlending(bool value);
		void SetBlendingFunction(GLenum source, GLenum destination);

		void ToggleFaceCulling(bool value);
		void SetCullFace(GLenum face);

		void SetPolygonMode(GLenum mode);

		//Switch shaders only if a different ID is requested. Not used too often.
		void SwitchShader(unsigned int ID);

	private:
		//Toggles
		bool m_DepthTestEnabled;
		bool m_BlendingEnabled;
		bool m_FaceCullingEnabled;

		//States
		GLenum m_DepthTestingFunction;
		GLenum m_BlendSource;
		GLenum m_BlendDestination;
		GLenum m_CulledFace;
		GLenum m_PolygonMode;

		//Shaders
		unsigned int m_ActiveShaderID;
	};
}