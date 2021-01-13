#include "CrescentPCH.h"
#include "PBR.h"
#include "Renderer.h"
#include "RenderTarget.h"
#include "Resources.h"
#include "EnvironmentalPBR.h"
#include "../Shading/Texture.h"
#include "../Shading/TextureCube.h"
#include "../Models/DefaultPrimitives.h"
#include "../Scene/SceneEntity.h"
#include "../Shading/Material.h"
#include "../Shading/Shader.h"

namespace Crescent
{
	PBR::PBR(Renderer* rendererContext)
	{
		m_RendererContext = rendererContext;

		m_RenderTargetBRDFLUT = new RenderTarget(128, 128, GL_HALF_FLOAT, 1, true);
		Shader* hdrToCubemapShader = Resources::LoadShader("HDR_To_Cubemap", "Resources/Shaders/PBR/CubeSampleVertex.shader", "Resources/Shaders/PBR/SphericalToCubeFragment.shader");
		Shader* irradianceCaptureShader = Resources::LoadShader("Irradiance", "Resources/Shaders/PBR/CubeSampleVertex.shader", "Resources/Shaders/PBR/IrradianceCaptureFragment.shader");
		Shader* prefilterCaptureShader = Resources::LoadShader("Prefilter", "Resources/Shaders/PBR/CubeSampleVertex.shader", "Resources/Shaders/PBR/PrefilterCaptureFragment.shader");
		Shader* integrateBRDFShader = Resources::LoadShader("Integrate_BRDF", "Resources/Shaders/ScreenQuadVertex.shader", "Resources/Shaders/PBR/IntegrateBRDFFragment.shader");

		m_PBRHDRToCubemapMaterial = new Material(hdrToCubemapShader);
		m_PBRIrradianceCaptureMaterial = new Material(irradianceCaptureShader);
		m_PBRPrefilterCaptureMaterial = new Material(prefilterCaptureShader);
		m_PBRIntegrateBRDFMaterial = new Material(integrateBRDFShader);

		m_PBRHDRToCubemapMaterial->m_DepthTestFunction = GL_LEQUAL;
		m_PBRIrradianceCaptureMaterial->m_DepthTestFunction = GL_LEQUAL;
		m_PBRPrefilterCaptureMaterial->m_DepthTestFunction = GL_LEQUAL;

		m_PBRHDRToCubemapMaterial->m_FaceCullingEnabled = false;
		m_PBRIrradianceCaptureMaterial->m_FaceCullingEnabled = false;
		m_PBRPrefilterCaptureMaterial->m_FaceCullingEnabled = false;

		m_PBRCaptureCube = new Cube();
		m_SceneEnvironmentCube = new SceneEntity("Scene Environment Cube", 0);
		m_SceneEnvironmentCube->m_Mesh = m_PBRCaptureCube;
		m_SceneEnvironmentCube->m_Material = m_PBRHDRToCubemapMaterial;

		//BRDF Integration
		m_RendererContext->Blit(nullptr, m_RenderTargetBRDFLUT, m_PBRIntegrateBRDFMaterial);
	}

	PBR::~PBR()
	{
	}

	EnvironmentalPBR* PBR::ProcessEquirectangularMap(Texture* environmentalMap)
	{
		//Convert HDR Radiance Image to HDR Environment Cubemap
		m_SceneEnvironmentCube->m_Material = m_PBRHDRToCubemapMaterial;
		m_PBRHDRToCubemapMaterial->SetShaderTexture("environment", environmentalMap, 0);

		TextureCube hdrEnvironmentalMap;
		hdrEnvironmentalMap.DefaultInitialize(128, 128, GL_RGB, GL_FLOAT);
		m_RendererContext->RenderCubemap(m_SceneEnvironmentCube, &hdrEnvironmentalMap);

		return ProcessCubeMap(&hdrEnvironmentalMap);
	}

	EnvironmentalPBR* PBR::ProcessCubeMap(TextureCube* environmentCapture, bool prefilter)
	{
		EnvironmentalPBR* environmentProbe = new EnvironmentalPBR();

		//Irradiance
		environmentProbe->m_IrradianceTextureCube = new TextureCube();
		environmentProbe->m_IrradianceTextureCube->DefaultInitialize(32, 32, GL_RGB, GL_FLOAT);
		m_PBRIrradianceCaptureMaterial->SetShaderTextureCube("environment", environmentCapture, 0);
		m_SceneEnvironmentCube->m_Material = m_PBRIrradianceCaptureMaterial;
		m_RendererContext->RenderCubemap(m_SceneEnvironmentCube, environmentProbe->m_IrradianceTextureCube, glm::vec3(0.0f), 0);

		//Prefilter
		if (prefilter)
		{
			environmentProbe->m_PrefilteredTextureCube = new TextureCube();
			environmentProbe->m_PrefilteredTextureCube->m_TextureCubeMinificationFilter = GL_LINEAR_MIPMAP_LINEAR;
			environmentProbe->m_PrefilteredTextureCube->DefaultInitialize(128, 128, GL_RGB, GL_FLOAT, true);
			m_PBRPrefilterCaptureMaterial->SetShaderTextureCube("environment", environmentCapture, 0);
			m_SceneEnvironmentCube->m_Material = m_PBRPrefilterCaptureMaterial;

			//Calculate prefilter for multiple roughness levels.
			unsigned int maxMipmappingLevels = 5;
			for (unsigned int i = 0; i < maxMipmappingLevels; i++)
			{
				m_PBRPrefilterCaptureMaterial->SetShaderFloat("roughness", (float)i / (float)(maxMipmappingLevels - 1));
				m_RendererContext->RenderCubemap(m_SceneEnvironmentCube, environmentProbe->m_PrefilteredTextureCube, glm::vec3(0.0f), i);
			}
		}

		return environmentProbe;
	}

	void PBR::SetSkyCapture(EnvironmentalPBR* environmentCapture)
	{
		m_SkyCapture = environmentCapture;
	}

	EnvironmentalPBR* PBR::RetrieveSkyCapture()
	{
		return m_SkyCapture;
	}
}