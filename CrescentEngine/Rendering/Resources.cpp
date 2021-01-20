#include "CrescentPCH.h"
#include "Resources.h"
#include "../Memory/ShaderLoader.h"
#include "../Memory/TextureLoader.h"
#include "../Memory/MeshLoader.h"
#include "../Shading/Texture.h"
#include "../Shading/TextureCube.h"
#include "../Utilities/StringID.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneEntity.h"

namespace Crescent
{
	std::map<unsigned int, Shader> Resources::m_Shaders = std::map<unsigned int, Shader>();
	std::map<unsigned int, Texture> Resources::m_Textures = std::map<unsigned int, Texture>();
	std::map<unsigned int, TextureCube> Resources::m_TextureCubes = std::map<unsigned int, TextureCube>();
	std::map<unsigned int, SceneEntity*> Resources::m_SceneMeshes = std::map<unsigned int, SceneEntity*>();

	void Resources::InitializeResourceManager()
	{

	}

	void Resources::Clean()
	{
	}

	Shader* Resources::LoadShader(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		unsigned int stringID = SID(name);

		//If shader exists, return that handle.
		if (Resources::m_Shaders.find(stringID) != Resources::m_Shaders.end())
		{
			return &Resources::m_Shaders[stringID];
		}

		CrescentInfo("Loading Shader: " + name);
		Shader shader = ShaderLoader::LoadShader(name, vertexShaderPath, fragmentShaderPath);
		Resources::m_Shaders[stringID] = shader;
		CrescentInfo("Successfully loaded Shader: " + name);
		return &Resources::m_Shaders[stringID];
	}

	Shader* Resources::RetrieveShader(const std::string& name)
	{
		unsigned int stringID = SID(name);

		//If shader exists, return that handle.
		if (Resources::m_Shaders.find(stringID) != Resources::m_Shaders.end())
		{
			return &Resources::m_Shaders[stringID];
		}
		else
		{
			CrescentError("Requested Shader: " + name + " not found.");
			return nullptr;
		}
	}

	Texture* Resources::LoadTexture(const std::string& name, const std::string& filePath, GLenum textureTarget, GLenum textureFormat, bool srgb)
	{
		unsigned int stringID = SID(name);

		//If texture already exists, return that handle.
		if (Resources::m_Textures.find(stringID) != Resources::m_Textures.end())
		{
			return &Resources::m_Textures[stringID];
		}

		CrescentInfo("Loading texture file at: " + filePath + ".");

		Texture texture = TextureLoader::LoadTexture(filePath, textureTarget, textureFormat, srgb);

		CrescentInfo("Successfully loaded: " + filePath + ".");

		//Make sure that the texture was properly loaded.
		if (texture.m_TextureWidth > 0)
		{
			Resources::m_Textures[stringID] = texture;
			return &Resources::m_Textures[stringID];
		}
		else
		{
			return nullptr;
		}
	}

	Texture* Resources::LoadHDRTexture(const std::string& name, const std::string& filePath)
	{
		unsigned int stringID = SID(name);

		//If the texture already exists, return that handle.
		if (Resources::m_Textures.find(stringID) != Resources::m_Textures.end())
		{
			return &Resources::m_Textures[stringID];
		}

		CrescentLoad("Loading HDR Texture At: " + filePath);

		Texture texture = TextureLoader::LoadHDRTexture(filePath);
		//Make sure the texture gets properly loaded.
		if (texture.m_TextureWidth > 0)
		{
			CrescentInfo("Successfully loaded HDR Texture.");
			Resources::m_Textures[stringID] = texture;
			return &Resources::m_Textures[stringID];
		}
		else
		{
			return nullptr;
		}
	}

	Texture* Resources::RetrieveTexture(const std::string& name)
	{
		unsigned int stringID = SID(name);

		//If texture exists, return that handle.
		if (Resources::m_Textures.find(stringID) != Resources::m_Textures.end())
		{
			return &Resources::m_Textures[stringID];
		}
		else
		{
			CrescentError("Requested Texture: " + name + " not found.");
			return nullptr;
		}
	}

	TextureCube* Resources::LoadTextureCube(const std::string& name, const std::string& folderPath)
	{
		unsigned int stringID = SID(name);

		//If the texture already exists, we return that handle.
		if (Resources::m_TextureCubes.find(stringID) != Resources::m_TextureCubes.end())
		{
			return &Resources::m_TextureCubes[stringID];
		}

		TextureCube textureCube = TextureLoader::LoadTextureCube(folderPath);
		Resources::m_TextureCubes[stringID] = textureCube;
		return &Resources::m_TextureCubes[stringID];
	}

	TextureCube* Resources::RetrieveTextureCube(const std::string& name)
	{
		unsigned stringID = SID(name);

		//If the texture cube already exists, return that handle.
		if (Resources::m_TextureCubes.find(stringID) != Resources::m_TextureCubes.end())
		{
			return &Resources::m_TextureCubes[stringID];
		}
		else
		{
			CrescentError("Requested Texture Cube: " + name + " not found.");
			return nullptr;
		}
	}

	SceneEntity* Resources::LoadMesh(Renderer* rendererContext, Scene* sceneContext, const std::string& meshName, const std::string& filePath)
	{
		unsigned int stringID = SID(meshName);

		//Check if mesh exists.
		if (Resources::m_SceneMeshes.find(stringID) != Resources::m_SceneMeshes.end())
		{
			return sceneContext->ConstructNewEntity(Resources::m_SceneMeshes[stringID]);
		}

		SceneEntity* sceneEntity = MeshLoader::LoadMesh(rendererContext, filePath);
		Resources::m_SceneMeshes[stringID] = sceneEntity;

		return sceneContext->ConstructNewEntity(sceneEntity);
	}

	SceneEntity* Resources::RetrieveMesh(const std::string& meshName)
	{
		unsigned int stringID = SID(meshName);

		if (Resources::m_SceneMeshes.find(stringID) != Resources::m_SceneMeshes.end())
		{
			//return Scene::ConstructNewEntity(Resources::m_SceneMeshes[stringID]);
		}
		else
		{
			CrescentError("Requested Mesh: " + meshName + " not found.");
			return nullptr;
		}
	}
}