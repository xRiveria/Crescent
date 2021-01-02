#include "CrescentPCH.h";
#include "Renderer.h";

namespace Crescent
{
	Renderer::~Renderer()
	{

	}

	void Renderer::InitializeRenderer()
	{

		//Configure the default OpenGL State.
	}

	void Renderer::SetRenderViewportSize(unsigned int newWidth, unsigned int newHeight)
	{

	}

	glm::vec2 Renderer::RetrieveRenderViewportSize() const
	{
		return glm::vec2();
	}

















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

	Material* Renderer::CreateMaterial(const std::string& base)
	{
		return nullptr;
	}

	Material* Renderer::CreateCustomMaterial(Shader* shader)
	{
		return nullptr;
	}

	Material* Renderer::CreatePostProcessingMaterial(Shader* shader)
	{
		return nullptr;
	}

	PBRCapture* Renderer::GetSkyCapture()
	{
		return nullptr;
	}

	void Renderer::ClearBuffers()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}