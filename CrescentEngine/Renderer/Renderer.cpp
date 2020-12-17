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
}