#pragma once

namespace Crescent
{
	class Renderer;
	class EnvironmentalPBR;
	class Texture;
	class TextureCube;
	class RenderTarget;
	class Material;
	class Mesh;
	class SceneEntity;

	/*
		Manages and mains all render data and functionality related to the (main) deferred PBR pipeline.
		This includes (pre)proprocessing (captured) reflection data (solving the render equation integral) for use in later rendering.
	*/

	class PBR
	{
		friend Renderer;

	public:
		PBR(Renderer* rendererContext);
		~PBR();

		//Generates an irradiance and pre-filter map out of a 2D equirectangular map (preferably HDR).
		EnvironmentalPBR* ProcessEquirectangularMap(Texture* environmentalMap);

		//Generates an irradiance and pre-filter map out of a cubemap texture.
		EnvironmentalPBR* ProcessCubeMap(TextureCube* environmentCapture, bool prefilter = true);

		//Sets the combined irradiance/pre-filter global environment skylight.
		void SetSkyCapture(EnvironmentalPBR* environmentCapture);

		//Retrieves the environment skylight.
		EnvironmentalPBR* RetrieveSkyCapture();

	private:
		EnvironmentalPBR* m_SkyCapture;
		RenderTarget* m_RenderTargetBRDFLUT;

		//PBR Pre-Processing (Irradiance/Pre-Filter)
		Material* m_PBRHDRToCubemapMaterial;
		Material* m_PBRIrradianceCaptureMaterial;
		Material* m_PBRPrefilterCaptureMaterial;
		Material* m_PBRIntegrateBRDFMaterial;

		Mesh* m_PBRCaptureCube;
		SceneEntity* m_SceneEnvironmentCube;

		Renderer* m_RendererContext;
	};
}