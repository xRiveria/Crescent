#pragma once
#include <glm/glm.hpp>

namespace Crescent
{
	class Renderer;
	class Material;
	class Mesh;
	class RenderTarget;
	class Texture2D;
	class TextureCube;
	class PBRCapture;
	class SceneNode;
	class Shader;
	class PostProcessor;

	/*
		Manages and maintains all render data and functionality related to the (main) deferred PBR pipeline. This includes (pre)processing (captured) reflection data (solving
		the render equation integral) for use in later rendering.
	*/

	class PBR
	{
		friend Renderer;

	public:
		PBR(Renderer* renderer);
		~PBR();

		//Sets the combined irradiance/pre-filter global environment skylight.
		void SetSkyCapture(PBRCapture* pbrEnvironmentCapture);
		//Adds a processed PBR capture to the list of irradiance probes.
		void AddIrradianceProbe(PBRCapture* capture, glm::vec3 position, float radius);
		//Removes all irradiance probe entires from the global illumination grid.
		void ClearIrradianceProbes();
		//Generate an irradiance and pre-filter map out of a 2D equirectangular map (preferably HDR).
		PBRCapture* ProcessEquirectangular(Texture2D* environmentMap);
		//Generate an irradiance and pre-filter map out of a cubemap texture.
		PBRCapture* ProcessCube(TextureCube* capture, bool prefilter = true);
		//Retrieves the environment skylight.
		PBRCapture* GetSkyCapture();
		//Retrieves all pushed irradiance probes.
		std::vector<PBRCapture*> GetIrradianceProbes(glm::vec3 queryPosition, float queryRadius);

		//Render all reflection/irradiance probes for visualization/debugging.
		void RenderProbes(); //Renders all reflection/irradiance probes for visualization/debugging.

	private:
		std::vector<PBRCapture*> m_CaptureProbes;
		PBRCapture* m_SkyCapture;
		RenderTarget* m_RenderTargetBRDFLUT;

		//PBR Pre-Processing (Irradiance/Prefilter)
		Material* m_PBRHDRToCubemap;
		Material* m_PBRIrradianceCapture;
		Material* m_PBRPrefilterCapture;
		Material* m_PBRIntegrateBRDF;
		Mesh* m_PBRCaptureCube;
		SceneNode* m_SceneEnvironmentCube;

		//Irradiance Capture
		Shader* m_ProbeCaptureShader;
		Shader* m_ProbeCaptureBackgroundShader;

		//Debug
		Mesh* m_ProbeDebugSphere;
		Shader* m_ProbeDebugShader;

		Renderer* m_Renderer;
	};
}
