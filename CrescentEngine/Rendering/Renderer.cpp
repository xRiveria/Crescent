#include "CrescentPCH.h";
#include "Renderer.h";

namespace CrescentEngine
{
	void Renderer::InitializeOpenGL()
	{
		if (glewInit() != GLEW_OK)
		{
			CrescentError("Failed to initialize GLEW.")
		}
		CrescentInfo("Successfully initialized GLEW.");
	}

	void Renderer::ToggleDepthTesting(bool value)
	{
		value ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	}

	void Renderer::ToggleWireframeRendering(bool value)
	{
		value ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void Renderer::ToggleBlending(bool value)
	{
		value ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void Renderer::ToggleFaceCulling(bool value)
	{
		value ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	}

	void Renderer::ClearBuffers()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}