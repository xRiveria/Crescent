#include "CrescentPCH.h"
#include "MaterialLibrary.h"
#include "Resources.h"
#include "../Utilities/StringID.h"

namespace CrescentEngine
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

	Material* MaterialLibrary::CreateMaterial(std::string& base) //Default material.
	{
		auto foundMaterial = m_DefaultMaterials.find(SID(base));
		if (foundMaterial != m_DefaultMaterials.end())
		{

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

		m_DefaultMaterials[SID("Default")] = defaultMaterial;
		//Glass Material.



		//Alpha Blend Material.


		//Alpha Cut Material.
	}

	void MaterialLibrary::GenerateInternalMaterials(RenderTarget* gBuffer)
	{

	}
}
