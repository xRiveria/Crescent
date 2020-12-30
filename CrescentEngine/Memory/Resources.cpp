#include "CrescentPCH.h"
#include "Resources.h"
#include "../Utilities/StringID.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "MeshLoader.h"

namespace CrescentEngine
{
	std::map<unsigned int, Shader> Resources::m_Shaders				= std::map<unsigned int, Shader>();
	std::map<unsigned int, Texture2D> Resources::m_Textures			= std::map<unsigned int, Texture2D>();
	std::map<unsigned int, TextureCube> Resources::m_TextureCubes	= std::map<unsigned int, TextureCube>();
	std::map<unsigned int, SceneNode*> Resources::m_Meshes			= std::map<unsigned int, SceneNode*>();

	void Resources::InitializeResourceManager()
	{
		//Initialize default assets/resources that should always be avaliable, regardless of configuration.
		Texture2D placeholderTexture;
	}

	void Resources::Clean()
	{
	}

	Shader* Resources::LoadShader(const std::string& name, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, std::vector<std::string> defines)
	{
		unsigned int ID = StringID(name);

		//If shader already exists, return that handle.
		if (Resources::m_Shaders.find(ID) != Resources::m_Shaders.end())
		{
			return &Resources::m_Shaders[ID];
		}

		Shader shader = ShaderLoader::LoadShader(name, vertexShaderFilePath, fragmentShaderFilePath, defines);
		Resources::m_Shaders[ID] = shader;
		return &Resources::m_Shaders[ID];
	}

	Shader* Resources::RetrieveShader(const std::string& name)
	{
		unsigned int ID = StringID(name);

		//If shader exists, return that handle.
		if (Resources::m_Shaders.find(ID) != Resources::m_Shaders.end())
		{
			return &Resources::m_Shaders[ID];
		}
		else
		{
			CrescentWarn("Requested Shader: " + name + " not found.");
			return nullptr;
		}
	}

	Texture2D* Resources::LoadTexture(const std::string& name, const std::string& filePath, GLenum textureTarget, GLenum textureInternalFormat, bool srgb)
	{
		unsigned int ID = StringID(name);

		//If texture already exists, return that handle.
		if (Resources::m_Textures.find(ID) != Resources::m_Textures.end())
		{
			return &Resources::m_Textures[ID];
		}

		CrescentInfo("Loading texture file at: " + filePath + ".");

		Texture2D texture = TextureLoader::LoadTexture(filePath, textureTarget, textureInternalFormat, srgb);

		CrescentInfo("Successfully loaded: " + filePath + ".");

		//Make sure that the texture was properly loaded.
		if (texture.m_TextureWidth > 0)
		{
			Resources::m_Textures[ID] = texture;
			return &Resources::m_Textures[ID];
		}
		else
		{
			CrescentInfo("Texture at: " + filePath + " has a width of 0. Please double check.");
			return nullptr;
		}
	}

	Texture2D* Resources::RetrieveTexture(const std::string& name)
	{
		unsigned int ID = StringID(name);

		//If texture exists, return that handle.
		if (Resources::m_Textures.find(ID) != Resources::m_Textures.end())
		{
			return &Resources::m_Textures[ID];
		}
		else
		{
			CrescentWarn("Requested texture: " + name + " not found.");
			return nullptr;
		}
	}

	TextureCube* Resources::LoadTextureCube(const std::string& name, const std::string& texturesFolderPath)
	{
		unsigned int ID = StringID(name);

		//If texture cube already exists, return that handle.
		if (Resources::m_TextureCubes.find(ID) != Resources::m_TextureCubes.end())
		{
			return &Resources::m_TextureCubes[ID];
		}

		TextureCube textureCube = TextureLoader::LoadTextureCube(texturesFolderPath);
		Resources::m_TextureCubes[ID] = textureCube;
		return &Resources::m_TextureCubes[ID];

	}

	TextureCube* Resources::RetrieveTextureCube(const std::string name)
	{
		unsigned int ID = StringID(name);

		//If texture cube exists, return that handle.
		if (Resources::m_TextureCubes.find(ID) != Resources::m_TextureCubes.end())
		{
			return &Resources::m_TextureCubes[ID];
		}
		else
		{
			CrescentInfo("Requested texture cube: " + name + " not found!");
			return nullptr;
		}
	}

	Texture2D* Resources::LoadHDRTexture(const std::string& name, const std::string& filePath)
	{
		unsigned int ID = StringID(name);

		//If HDR texture exists, return that handle.
		if (Resources::m_Textures.find(ID) != Resources::m_Textures.end())
		{
			return &Resources::m_Textures[ID];
		}

		Texture2D texture = TextureLoader::LoadHDRTexture(filePath);

		//Make sure texture gets properly loaded.
		if (texture.m_TextureWidth > 0)
		{
			Resources::m_Textures[ID] = texture;
			return &Resources::m_Textures[ID];
		}
		else
		{
			return nullptr;
		}
	}

	SceneNode* Resources::LoadMesh(Renderer* renderer, const std::string& name, const std::string& filePath)
	{
		unsigned int ID = StringID(name);

		//If a mesh's scene entity was already loaded before, copy the scene entity's memory and return the copied reference. We return a copy as the moment the global scene deletes the
		//returned entity, all other and next requested scene entities of this model will end up as dangling pointers.
		if (Resources::m_Meshes.find(ID) != Resources::m_Meshes.end())
		{
			
		}
	}

	SceneNode* Resources::RetrieveMesh(const std::string& name)
	{
		return nullptr;
	}

	
}