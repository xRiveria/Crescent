#pragma once

namespace Crescent
{
	/*
		Maintains and manages all data and functionality related to end-of-frame post-processing. This doesn't just include post-processing effects like
		SSAO, Bloom, Vignette etc, but also includes general functionality like blitting, blurring, HDR and Gamma Correction.
	*/

	class RenderTarget;
	class Renderer;
	class Shader;
	class Texture;
	class Camera;

	class PostProcessor
	{
	public:
		PostProcessor(Renderer* rendererContext);
		~PostProcessor();

		//Updates all render targets to match the new render size.
		void UpdatePostProcessingRenderTargetSizes(unsigned int newWidth, unsigned int newHeight);

		//Process Stages
		void ProcessPreLighting(Renderer* rendererContext, RenderTarget* gBuffer, Camera* cameraContext);
		void ProcessPostLighting(Renderer* rendererContext, RenderTarget* gBuffer, RenderTarget& outputRenderTarget, Camera* cameraContext);

		//Blit all combined post-processing steps to our default framebuffer.
		void BlitToMainFramebuffer(Renderer* rendererContext, Texture* sourceTexture);

	public:
		bool m_SSAOEnabled = true;
		bool m_BloomEnabled = true;

		bool m_InversionEnabled = false;
		bool m_GreyscaleEnabled = false;

		int SSAOKernelSize = 32;
		Texture* m_SSAOOutput;

		Shader* m_PostProcessingShader;

	private:
		//SSAO
		RenderTarget* m_SSAORenderTarget;
		Shader* m_SSAOShader;
		Shader* m_SSAOBlurShader;
		Texture* m_SSAONoiseTexture;

		//Bloom
		Shader* m_BloomShader;
		Shader* m_BloomBlurShader;

		RenderTarget* m_BloomRenderTarget0;
		RenderTarget* m_BloomRenderTarget1;
		RenderTarget* m_BloomRenderTarget2;
		RenderTarget* m_BloomRenderTarget3;
		RenderTarget* m_BloomRenderTarget4;

		Texture* m_BloomOutput1;
		Texture* m_BloomOutput2;
		Texture* m_BloomOutput3;
		Texture* m_BloomOutput4;

		Renderer* m_Renderer;
	};
}