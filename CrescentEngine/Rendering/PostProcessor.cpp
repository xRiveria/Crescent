#include "CrescentPCH.h"
#include "PostProcessor.h"
#include "Resources.h"
#include "../Shading/Shader.h"
#include "../Rendering/RenderTarget.h"
#include "../Utilities/Camera.h"
#include "../Models/DefaultPrimitives.h"
#include "Renderer.h"
#include <random>
#include "glm/gtx/compatibility.hpp"

namespace Crescent
{
	PostProcessor::PostProcessor(Renderer* rendererContext)
	{
		m_PostProcessingShader = Resources::LoadShader("Post Process", "Resources/Shaders/ScreenQuadVertex.shader", "Resources/Shaders/PostProcessingFragment.shader");
		m_PostProcessingShader->UseShader();
		m_PostProcessingShader->SetUniformInteger("TexSrc", 0);
		m_PostProcessingShader->SetUniformInteger("TexBloom1", 1);
		m_PostProcessingShader->SetUniformInteger("TexBloom2", 2);
		m_PostProcessingShader->SetUniformInteger("TexBloom3", 3);
		m_PostProcessingShader->SetUniformInteger("TexBloom4", 4);
		m_PostProcessingShader->SetUniformInteger("gMotion", 5);

		//SSAO
		m_SSAORenderTarget = new RenderTarget(1280, 720, GL_HALF_FLOAT, 1, false);
		m_SSAOOutput = m_SSAORenderTarget->RetrieveColorAttachment(0);

		m_SSAOShader = Resources::LoadShader("SSAO", "Resources/Shaders/ScreenQuadVertex.shader", "Resources/Shaders/Post/SSAOFragment.shader");
		m_SSAOShader->UseShader();
		m_SSAOShader->SetUniformInteger("gPositionMetallic", 0);
		m_SSAOShader->SetUniformInteger("gNormalRoughness", 1);
		m_SSAOShader->SetUniformInteger("texNoise", 2);

		std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
		std::default_random_engine generator;
		std::vector<glm::vec3> ssaoKernel;

		for (int i = 0; i < SSAOKernelSize; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0f - 1.0f,
				randomFloats(generator) * 2.0f - 1.0f,
				randomFloats(generator)
			);
			sample = glm::normalize(sample);
			sample = sample * randomFloats(generator);
			float scale = (float)i / (float)SSAOKernelSize;
			scale = glm::lerp(0.1f, 1.0f, scale * scale);
			sample = sample * scale;
			ssaoKernel.push_back(sample);
		}

		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		m_SSAONoiseTexture = new Texture();
		m_SSAONoiseTexture->GenerateTexture(4, 4, GL_RGBA16F, GL_RGB, GL_HALF_FLOAT, &ssaoNoise[0]);

		m_SSAOShader->SetUniformVectorArray("kernel", ssaoKernel.size(), ssaoKernel);
		m_SSAOShader->SetUniformInteger("sampleCount", SSAOKernelSize);
		m_SSAOShader->SetUniformInteger("HDRScene", 0);

		//Bloom
		m_BloomRenderTarget0 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
		m_BloomRenderTarget1 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
		m_BloomRenderTarget2 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
		m_BloomRenderTarget3 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
		m_BloomRenderTarget4 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);

		m_BloomOutput1 = m_BloomRenderTarget1->RetrieveColorAttachment(0);
		m_BloomOutput2 = m_BloomRenderTarget1->RetrieveColorAttachment(0);
		m_BloomOutput3 = m_BloomRenderTarget1->RetrieveColorAttachment(0);
		m_BloomOutput4 = m_BloomRenderTarget1->RetrieveColorAttachment(0);

		//m_BloomShader = Resources::LoadShader("Bloom", "Resources/Shaders/ScreenQuadVertex.shader", "Resources/Shaders/Post/BloomFragment.shader");
	}

	PostProcessor::~PostProcessor()
	{
	}

	void PostProcessor::UpdatePostProcessingRenderTargetSizes(unsigned int newWidth, unsigned int newHeight)
	{
		m_SSAORenderTarget->ResizeRenderTarget((int)newWidth * 0.5f, (int)(newHeight * 0.5f));

		m_BloomRenderTarget0->ResizeRenderTarget((int)newWidth * 0.5f, (int)(newHeight * 0.5f));
		m_BloomRenderTarget1->ResizeRenderTarget((int)newWidth * 0.5f, (int)(newHeight * 0.5f));
		m_BloomRenderTarget2->ResizeRenderTarget((int)newWidth * 0.25f, (int)(newHeight * 0.25f));
		m_BloomRenderTarget3->ResizeRenderTarget((int)newWidth * 0.125f, (int)(newHeight * 0.125f));
		m_BloomRenderTarget4->ResizeRenderTarget((int)newWidth * 0.0675f, (int)(newHeight * 0.0675f));
	}

	void PostProcessor::ProcessPreLighting(Renderer* rendererContext, RenderTarget* gBuffer, Camera* cameraContext)
	{
		if (m_SSAOEnabled)
		{
			gBuffer->RetrieveColorAttachment(0)->BindTexture(0);
			gBuffer->RetrieveColorAttachment(1)->BindTexture(1);
			m_SSAONoiseTexture->BindTexture(2);

			m_SSAOShader->UseShader();
			m_SSAOShader->SetUniformVector2("renderSize", rendererContext->RetrieveRenderWindowSize());
			m_SSAOShader->SetUniformMat4("projection", cameraContext->m_ProjectionMatrix);
			m_SSAOShader->SetUniformMat4("view", cameraContext->m_ViewMatrix);

			glBindFramebuffer(GL_FRAMEBUFFER, m_SSAORenderTarget->m_FramebufferID);
			glViewport(0, 0, m_SSAORenderTarget->m_FramebufferWidth, m_SSAORenderTarget->m_FramebufferHeight);
			glClear(GL_COLOR_BUFFER_BIT);
			rendererContext->RenderMesh(rendererContext->m_NDCQuad);
		}
	}

	void PostProcessor::ProcessPostLighting(Renderer* rendererContext, RenderTarget* gBuffer, RenderTarget& outputRenderTarget, Camera* cameraContext)
	{

	}

	void PostProcessor::BlitToMainFramebuffer(Renderer* rendererContext, Texture* sourceTexture)
	{

	}
}