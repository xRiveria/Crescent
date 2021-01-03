#include "CrescentPCH.h"
#include "PBR.h"
#include "Renderer.h"
#include "RenderTarget.h"
#include "PBRCapture.h"
#include "../Memory/Resources.h"
#include "../Models/DefaultShapes.h"
#include "Scene/Scene.h"
#include "Scene/SceneNode.h"
#include "Material.h"
#include "Shader.h"
#include "../Utilities/Camera.h"
#include <memory>

namespace Crescent
{
	PBR::PBR(Renderer* renderer)
	{
		m_Renderer = renderer;

		m_RenderTargetBRDFLUT = new RenderTarget(128, 128, GL_HALF_FLOAT, 1, true);
		Shader* hdrToCubemap = Resources::LoadShader("PBR:HDRToCubemap", "Resources/Shaders/PBR/CubeSample.vs", "Resources/Shaders/PBR/SphericalToCube.fs");
		Shader* irradianceCapture = Resources::LoadShader("PBR:Irradiance", "Resources/Shaders/PBR/CubeSample.vs", "Resources/Shaders/PBR/IrradianceCapture.fs");
		Shader* prefilterCapture = Resources::LoadShader("PBR:Prefilter", "Resources/Shaders/PBR/CubeSample.vs", "Resources/Shaders/PBR/PrefilterCapture.fs");
		Shader* integrateBRDF = Resources::LoadShader("PBR:IntegrateBRDF", "Resources/Shaders/ScreenQuad.vs", "Resources/Shaders/PBR/IntegrateBRDF.fs");

		m_PBRHDRToCubemap = new Material(hdrToCubemap);
		m_PBRIrradianceCapture = new Material(irradianceCapture);
		m_PBRPrefilterCapture = new Material(prefilterCapture);
		m_PBRIntegrateBRDF = new Material(integrateBRDF);

		m_PBRHDRToCubemap->m_DepthTestComparisonFactor = GL_LEQUAL;
		m_PBRIrradianceCapture->m_DepthTestComparisonFactor = GL_LEQUAL;
		m_PBRPrefilterCapture->m_DepthTestComparisonFactor = GL_LEQUAL;

		m_PBRHDRToCubemap->m_FaceCullingEnabled = false;
		m_PBRIrradianceCapture->m_FaceCullingEnabled = false;
		m_PBRPrefilterCapture->m_FaceCullingEnabled = false;

		m_PBRCaptureCube = new Cube();
		m_SceneEnvironmentCube = new SceneNode(0);
		m_SceneEnvironmentCube->m_Mesh = m_PBRCaptureCube;
		m_SceneEnvironmentCube->m_Material = m_PBRHDRToCubemap;

		//BRDF Integration
		m_Renderer->Blit(nullptr, m_RenderTargetBRDFLUT, m_PBRIntegrateBRDF);

		//Capture
		m_ProbeCaptureShader = Resources::LoadShader("PBR:Capture", "Resources/Shaders/Capture.vs", "Resources/Shaders/Capture.fs");
		m_ProbeCaptureShader->UseShader();
		m_ProbeCaptureShader->SetUniformInteger("TexAlbedo", 0);
		m_ProbeCaptureShader->SetUniformInteger("TexNormal", 1);
		m_ProbeCaptureShader->SetUniformInteger("TexMetallic", 2);
		m_ProbeCaptureShader->SetUniformInteger("TexRoughness", 3);

		m_ProbeCaptureBackgroundShader = Resources::LoadShader("PBR:CaptureBackground", "Resources/Shaders/PBR/ProbeRender.vs", "Resources/Shaders/PBR/ProbeRender.fs");
		m_ProbeCaptureBackgroundShader->UseShader();
		m_ProbeCaptureBackgroundShader->SetUniformInteger("background", 0);
		
		//Debug Render
		m_ProbeDebugSphere = new Sphere(32, 32);
		m_ProbeDebugShader = Resources::LoadShader("PBR:ProbeRender", "Resources/Shaders/PBR/ProbeRender.vs", "Resources/Shaders/PBR/ProbeRender.fs");
		m_ProbeDebugShader->UseShader();
		m_ProbeDebugShader->SetUniformInteger("PrefilterMap", 0);
	}

	PBR::~PBR()
	{
		delete m_PBRCaptureCube;
		delete m_SceneEnvironmentCube;
		delete m_RenderTargetBRDFLUT;
		delete m_PBRHDRToCubemap;
		delete m_PBRIrradianceCapture;
		delete m_PBRPrefilterCapture;
		delete m_PBRIntegrateBRDF;
		delete m_SkyCapture;

		for (int i = 0; i < m_CaptureProbes.size(); ++i)
		{
			delete m_CaptureProbes[i]->m_Irradiance;
			delete m_CaptureProbes[i]->m_Prefiltered;
			delete m_CaptureProbes[i];
		}
		delete m_ProbeDebugSphere;
	}

	void PBR::SetSkyCapture(PBRCapture* pbrEnvironmentCapture)
	{
		m_SkyCapture = pbrEnvironmentCapture;
	}

	void PBR::AddIrradianceProbe(PBRCapture* capture, glm::vec3 position, float radius)
	{
		capture->m_Position = position;
		capture->m_Radius = radius;
		m_CaptureProbes.push_back(capture);
	}

	void PBR::ClearIrradianceProbes()
	{
		for (int i = 0; i < m_CaptureProbes.size(); ++i)
		{
			delete m_CaptureProbes[i]->m_Irradiance;
			delete m_CaptureProbes[i]->m_Prefiltered;
			delete m_CaptureProbes[i];
		}
		m_CaptureProbes.clear();
	}

	PBRCapture* PBR::GetSkyCapture()
	{
		return m_SkyCapture;
	}

	std::vector<PBRCapture*> PBR::GetIrradianceProbes(glm::vec3 queryPosition, float queryRadius)
	{
		//Retrieve all irradiance probes in proximity to queryPosition and queryRadius.
		std::vector<PBRCapture*> capturesInProximity;
		for (int i = 0; i < m_CaptureProbes.size(); ++i)
		{
			//To implement.
		}
	}

	PBRCapture* PBR::ProcessEquirectangular(Texture2D* environmentMap)
	{
		//Convert HDR radiance image to HDR environment cubemap.
		m_SceneEnvironmentCube->m_Material = m_PBRHDRToCubemap;
		m_PBRHDRToCubemap->SetShaderTexture("environment", environmentMap, 0);
		TextureCube hdrEnvironmentMap;
		hdrEnvironmentMap.DefaultInitializeCubemapTexture(128, 128, GL_RGB, GL_FLOAT);
		m_Renderer->RenderToCubemap(m_SceneEnvironmentCube, &hdrEnvironmentMap);

		return ProcessCube(&hdrEnvironmentMap);
	}

	PBRCapture* PBR::ProcessCube(TextureCube* capture, bool prefilter)
	{
		PBRCapture* captureProbe = new PBRCapture;

		//Irradiance
		captureProbe->m_Irradiance = new TextureCube;
		captureProbe->m_Irradiance->DefaultInitializeCubemapTexture(32, 32, GL_RGB, GL_FLOAT);
		m_PBRIrradianceCapture->SetShaderTextureCube("environment", capture, 0);
		m_SceneEnvironmentCube->m_Material = m_PBRIrradianceCapture;
		m_Renderer->RenderToCubemap(m_SceneEnvironmentCube, captureProbe->m_Irradiance, glm::vec3(0.0f), 0);

		//Prefilter
		if (prefilter)
		{
			captureProbe->m_Prefiltered = new TextureCube;
			captureProbe->m_Prefiltered->m_MinificationFilter = GL_LINEAR_MIPMAP_LINEAR;
			captureProbe->m_Prefiltered->DefaultInitializeCubemapTexture(128, 128, GL_RGB, GL_FLOAT, true);
			m_PBRPrefilterCapture->SetShaderTextureCube("environment", capture, 0);
			m_SceneEnvironmentCube->m_Material = m_PBRPrefilterCapture;
			//Calculate Prefilter for multiple roughness levels.
			unsigned int maxMipmapLevels = 5;
			for (unsigned int i = 0; i < maxMipmapLevels; ++i)
			{
				m_PBRPrefilterCapture->SetShaderFloat("roughness", (float)i / (float)(maxMipmapLevels - 1));
				m_Renderer->RenderToCubemap(m_SceneEnvironmentCube, captureProbe->m_Prefiltered, glm::vec3(0.0f), i);
			}
		}

		return captureProbe;
	}

	void PBR::RenderProbes()
	{
		m_ProbeDebugShader->UseShader();
		m_ProbeDebugShader->SetUniformMat4("projection", m_Renderer->RetrieveCamera()->m_ProjectionMatrix);
		m_ProbeDebugShader->SetUniformMat4("view", m_Renderer->RetrieveCamera()->m_ViewMatrix);
		m_ProbeDebugShader->SetUniformVector3("cameraPosition", m_Renderer->RetrieveCamera()->m_CameraPosition);

		//First, render the sky capture.
		m_ProbeDebugShader->SetUniformVector3("Position", glm::vec3(0.0f, 2.0f, 0.0f));
		m_SkyCapture->m_Prefiltered->BindCubemapTexture(0);
		m_Renderer->RenderMesh(m_ProbeDebugSphere, m_ProbeDebugShader);

		//Then, do the same for each capture probe at their respective locations.
		for (int i = 0; i < m_CaptureProbes.size(); ++i)
		{
			m_ProbeDebugShader->SetUniformVector3("Position", m_CaptureProbes[i]->m_Position);
			if (m_CaptureProbes[i]->m_Prefiltered)
			{
				m_CaptureProbes[i]->m_Prefiltered->BindCubemapTexture(0);
			}
			else
			{
				m_CaptureProbes[i]->m_Irradiance->BindCubemapTexture(0);
			}
			m_Renderer->RenderMesh(m_ProbeDebugSphere, m_ProbeDebugShader);
		}
	}
}