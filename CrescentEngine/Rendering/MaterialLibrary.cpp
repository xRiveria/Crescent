#include "CrescentPCH.h"
#include "MaterialLibrary.h"
#include "../Shading/Material.h"
#include "Resources.h"
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

		//delete m_DebugLightMaterial;
		//delete m_DefaultBlitMaterial;
	}

	Material* MaterialLibrary::CreateMaterial(std::string& base) //Default material.
	{
		auto foundMaterial = m_DefaultMaterials.find(SID(base));
		if (foundMaterial != m_DefaultMaterials.end())
		{
			Material copy = foundMaterial->second->CopyMaterial(); 
			Material* material = new Material(copy);
			m_Materials.push_back(material);
			return material;
		}
		else
		{
			CrescentInfo("Material of template: " + base + " requested, but template does not exist.");
			return nullptr;
		}
	}

	void MaterialLibrary::GenerateDefaultMaterials()
	{
		//Default render material (deferred path).
		Shader* defaultShader = Resources::LoadShader("Default", "Resources/Shaders/Deferred/GBufferVertex.shader", "Resources/Shaders/Deferred/GBufferFragment.shader");
		Material* defaultMaterial = new Material(defaultShader);

		defaultMaterial->m_MaterialType = Material_Default;
		defaultMaterial->SetShaderTexture("TexAlbedo", Resources::LoadTexture("Default Albedo", "Resources/Textures/Checkerboard.png", GL_TEXTURE_2D, GL_RGB), 3);
		//defaultMaterial->SetShaderTexture("TexNormal", Resources::LoadTexture("Default Normal", "Resources/Textures/Normal.png"), 4);
		//defaultMaterial->SetShaderTexture("TexMetallic", Resources::LoadTexture("Default Metallic", "Resouces/Textures/Black.png"), 5);
		//defaultMaterial->SetShaderTexture("TexRoughness", Resources::LoadTexture("Default Roughness", "Resouces/Textures/Checkboard.png"), 6);

		m_DefaultMaterials[SID("Default")] = defaultMaterial;
	}

	void MaterialLibrary::GenerateInternalMaterials(RenderTarget* gBuffer)
	{
		//Deferred
		m_DeferredDirectionalLightShader = Resources::LoadShader("Deferred Directional Light", "Resources/Shaders/Deferred/ScreenDirectionalVertex.shader", "Resources/Shaders/Deferred/DirectionalFragment.shader");
		m_DeferredPointLightShader = Resources::LoadShader("Deferred Point Light", "Resources/Shaders/Deferred/PointLightVertex.shader", "Resources/Shaders/Deferred/PointLightFragment.shader");
			
		//Take in from GBuffer color buffers.
		m_DeferredDirectionalLightShader->UseShader();
		m_DeferredDirectionalLightShader->SetUniformInteger("gPosition", 0);
		m_DeferredDirectionalLightShader->SetUniformInteger("gNormal", 1);
		m_DeferredDirectionalLightShader->SetUniformInteger("gAlbedo", 2);
		m_DeferredDirectionalLightShader->SetUniformInteger("lightShadowMap", 3);

		m_DeferredPointLightShader->UseShader();
		m_DeferredPointLightShader->SetUniformInteger("gPosition", 0);
		m_DeferredPointLightShader->SetUniformInteger("gNormal", 1);
		m_DeferredPointLightShader->SetUniformInteger("gAlbedo", 2);

		//Shadows
		m_DirectionalShadowShader = Resources::LoadShader("Directional Shadow", "Resources/Shaders/ShadowCastVertex.shader", "Resources/Shaders/ShadowCastFragment.shader");

		//Debug
		Shader* debugLightShader = Resources::LoadShader("Debug Light", "Resources/Shaders/LightDebugVertex.shader", "Resources/Shaders/LightDebugFragment.shader");
		m_DebugLightMaterial = new Material(debugLightShader);
	}
/*
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

	void MaterialLibrary::GenerateInternalMaterials(RenderTarget* gBuffer)
	{

	}
*/
}
