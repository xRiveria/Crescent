#include "CrescentPCH.h"
#include "MaterialLibrary.h"
#include "../Memory/Resources.h"
#include "../Utilities/StringID.h"

namespace Crescent
{
	MaterialLibrary::MaterialLibrary(RenderTarget* gBuffer)
	{
		GenerateDefaultMaterials();
		GenerateInternalMaterials(gBuffer);
	}

	MaterialLibrary::~MaterialLibrary()
	{
		for (auto iterator = m_DefaultMaterials.begin(); iterator != m_DefaultMaterials.end(); ++iterator)
		{
			delete iterator->second;
		}

		for (unsigned int i = 0; i < m_Materials.size(); ++i)
		{
			delete m_Materials[i];
		}

		delete m_DebugLightMaterial;
		delete m_DefaultBlitMaterial;
	}

	Material* MaterialLibrary::CreateMaterial(const std::string& base) //Default material.
	{
		auto foundMaterial = m_DefaultMaterials.find(StringID(base));
		if (foundMaterial != m_DefaultMaterials.end())
		{
			Material copiedMaterial = foundMaterial->second->Copy();
			Material* material = new Material(copiedMaterial);
			m_Materials.push_back(material);

			return material;
		}
		else
		{
			CrescentInfo("Material of template: " + base + " requested, but template does not exist.");
			return nullptr;
		}
	}

	Material* MaterialLibrary::CreateCustomMaterial(Shader* shader) //Player created.
	{
		Material* material = new Material(shader);
		material->m_MaterialType = Material_Custom;
		m_Materials.push_back(material);

		return material;
	}

	Material* MaterialLibrary::CreatePostProcessingMaterial(Shader* shader) 
	{
		Material* material = new Material(shader);
		material->m_MaterialType = Material_PostProcess;
		m_Materials.push_back(material);

		return material;
	}

	void MaterialLibrary::GenerateDefaultMaterials()
	{
		//Default render material (deferred path).
		Shader* defaultShader = Resources::LoadShader("Default", "Resources/Shaders/Deferred/GBuffer.vs", "Resources/Shaders/GBuffer.fs");
		
		Material* defaultMaterial = new Material(defaultShader);
		defaultMaterial->m_MaterialType = Material_Default;
		defaultMaterial->SetShaderTexture("TexAlbedo", Resources::LoadTexture("Default Albedo", "Resources/Textures/Checkerboard.png", GL_TEXTURE_2D, GL_RGB), 3);
		defaultMaterial->SetShaderTexture("TexNormal", Resources::LoadTexture("Default Normal", "Resources/Textures/Normal.png"), 4);
		defaultMaterial->SetShaderTexture("TexMetallic", Resources::LoadTexture("Default Metallic", "Resouces/Textures/Black.png"), 5);
		defaultMaterial->SetShaderTexture("TexRoughness", Resources::LoadTexture("Default Roughness", "Resouces/Textures/Checkboard.png"), 6);

		m_DefaultMaterials[StringID("Default")] = defaultMaterial;

		//Glass Material.
		Shader* glassShader = Resources::LoadShader("Glass", "Resources/Shaders/ForwardRender.vs", "Resouces/Shaders/ForwardRender.fs", { "ALPHA_BLEND" });
		glassShader->UseShader();
		glassShader->SetUniformInteger("lightShadowMap1", 10);
		glassShader->SetUniformInteger("lightShadowMap2", 10);
		glassShader->SetUniformInteger("lightShadowMap3", 10);
		glassShader->SetUniformInteger("lightShadowMap4", 10);

		Material* glassMaterial = new Material(glassShader);
		glassMaterial->m_MaterialType = Material_Custom; //This material cannot fit into the deferred rendering pipeline due to blending. 
		glassMaterial->SetShaderTexture("TexAlbedo", Resources::LoadTexture("Glass Albedo", "Resources/Textures/Glass.png", GL_TEXTURE_2D, GL_RGBA), 0);
		glassMaterial->SetShaderTexture("TexNormal", Resources::LoadTexture("Glass Normal", "Resources/Textures/PBR/Plastic/Normal.png"), 1);
		glassMaterial->SetShaderTexture("TexMetallic", Resources::LoadTexture("Glass Metallic", "Resources/Textures/PBR/Plastic/Metallic.png"), 2);
		glassMaterial->SetShaderTexture("TexRoughness", Resources::LoadTexture("Glass Roughness", "Resources/Textures/PBR/Plastic/Roughness.png"), 3);
		glassMaterial->SetShaderTexture("TexAO", Resources::LoadTexture("Glass AO", "Resources/Textures/PBR/Plastic/AO.png"), 4);
		glassMaterial->m_BlendingEnabled = true;
		m_DefaultMaterials[StringID("Glass")] = glassMaterial;

		//Alpha Blend Material.
		Shader* alphaBlendShader = Resources::LoadShader("Alpha Blend", "Resources/Shaders/ForwardRender.vs", "Resources/Shaders/ForwardRender.fs", { "ALPHA_BLEND" });
		alphaBlendShader->UseShader();
		alphaBlendShader->SetUniformInteger("lightShadowMap1", 10);
		alphaBlendShader->SetUniformInteger("lightShadowMap2", 10);
		alphaBlendShader->SetUniformInteger("lightShadowMap3", 10);
		alphaBlendShader->SetUniformInteger("lightShadowMap4", 10);
		
		Material* alphaBlendMaterial = new Material(alphaBlendShader);
		alphaBlendMaterial->m_MaterialType = Material_Custom;
		alphaBlendMaterial->m_BlendingEnabled = true;
		m_DefaultMaterials[StringID("Alpha Blend")] = alphaBlendMaterial;

		//Alpha Cutout Material.
		Shader* alphaDiscardShader = Resources::LoadShader("Alpha Discard", "Resources/Shaders/ForwardRender.fs", "Resources/Shaders/ForwardRender.vs", { "ALPHA_DISCARD" });
		alphaDiscardShader->UseShader();
		alphaDiscardShader->SetUniformInteger("lightShadowMap1", 10);
		alphaDiscardShader->SetUniformInteger("lightShadowMap2", 10);
		alphaDiscardShader->SetUniformInteger("lightShadowMap3", 10);
		alphaDiscardShader->SetUniformInteger("lightShadowMap4", 10);

		Material* alphaDiscardMaterial = new Material(alphaDiscardShader);
		alphaDiscardMaterial->m_MaterialType = Material_Custom;
		alphaDiscardMaterial->m_FaceCullingEnabled = false;
		m_DefaultMaterials[StringID("Alpha Discard")] = alphaDiscardMaterial;

	}

	void MaterialLibrary::GenerateInternalMaterials(RenderTarget* gBuffer)
	{
		//Post-Processing
		Shader* defaultBlitShader = Resources::LoadShader("Blit", "Resources/Shaders/Screen_Quad.vs", "Resouces/Shaders/Default_Blit.fs");
		m_DefaultBlitMaterial = new Material(defaultBlitShader);

		//Deferred
		m_DeferredAmbientLightShader = Resources::LoadShader("Deferred Ambient Light", "Resources/Shaders/Deferred/ScreenAmbient.vs", "Resources/Shaders/Deferred/Ambient.fs");
		m_DeferredIrradianceShader = Resources::LoadShader("Deferred Irradiance", "Resources/Shaders/Deferred/AmbientIrradiance.vs", "Resources/Shaders/Deferred/Ambient_Irradiance.fs");
		m_DeferredDirectionalLightShader = Resources::LoadShader("Deferred Directional Light", "Resources/Shaders/Deferred/ScreenDirectional.vs", "Resouces/Shaders/Deferred/Directional.fs");
		m_DeferredPointLightShader = Resources::LoadShader("Deferred Point Light", "Resources/Shaders/Deferred/Point.vs", "Resources/Shaders/Deferred/Point.fs");

		m_DeferredAmbientLightShader->UseShader();
		m_DeferredAmbientLightShader->SetUniformInteger("gPositionMetallic", 0);
		m_DeferredAmbientLightShader->SetUniformInteger("gNormalRoughness", 1);
		m_DeferredAmbientLightShader->SetUniformInteger("gAlbedoAO", 2);
		m_DeferredAmbientLightShader->SetUniformInteger("environmentIrradiance", 3);
		m_DeferredAmbientLightShader->SetUniformInteger("environmentPrefilter", 4);
		m_DeferredAmbientLightShader->SetUniformInteger("BRDFLUT", 5);
		m_DeferredAmbientLightShader->SetUniformInteger("TexSSAO", 6);

		m_DeferredIrradianceShader->UseShader();
		m_DeferredIrradianceShader->SetUniformInteger("gPositionMetallic", 0);
		m_DeferredIrradianceShader->SetUniformInteger("gNormalRoughness", 1);
		m_DeferredIrradianceShader->SetUniformInteger("gAlbedoAO", 2);
		m_DeferredIrradianceShader->SetUniformInteger("environmentIrradiance", 3);
		m_DeferredIrradianceShader->SetUniformInteger("environmentPrefilter", 4);
		m_DeferredIrradianceShader->SetUniformInteger("BRDFLUT", 5);
		m_DeferredIrradianceShader->SetUniformInteger("TexSSAO", 6);

		m_DeferredDirectionalLightShader->UseShader();
		m_DeferredDirectionalLightShader->SetUniformInteger("gPositionMetallic", 0);
		m_DeferredDirectionalLightShader->SetUniformInteger("gNormalRoughness", 1);
		m_DeferredDirectionalLightShader->SetUniformInteger("gAlbedoAO", 2);
		m_DeferredDirectionalLightShader->SetUniformInteger("lightShaderMap", 3);

		m_DeferredPointLightShader->UseShader();
		m_DeferredPointLightShader->SetUniformInteger("gPositionMetallic", 0);
		m_DeferredPointLightShader->SetUniformInteger("gNormalRoughness", 1);
		m_DeferredPointLightShader->SetUniformInteger("gAlbedoAO", 2);

		//Shadows
		m_DirectionalShadowShader = Resources::LoadShader("Shadow Directional", "Resources/Shaders/ShadowCast.vs", "Resources/Shadows/ShadowCast.fs");

		//Debug
		Shader* debugLightShader = Resources::LoadShader("Debug Light", "Resources/Shaders/Light.vs", "Resources/Shaders/Light.fs");
		m_DebugLightMaterial = new Material(debugLightShader);
	}
}
