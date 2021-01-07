#include "CrescentPCH.h"
#include "Resources.h"
#include "../Memory/ShaderLoader.h"
#include "../Memory/TextureLoader.h"
#include "../Shading/Texture.h"
#include "../Utilities/StringID.h"

namespace Crescent
{
	std::map<unsigned int, Shader> Resources::m_Shaders = std::map<unsigned int, Shader>();
	std::map<unsigned int, Texture> Resources::m_Textures = std::map<unsigned int, Texture>();

	void Resources::InitializeResourceManager()
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

		Shader shader = ShaderLoader::LoadShader(name, vertexShaderPath, fragmentShaderPath);
		Resources::m_Shaders[stringID] = shader;
		return &Resources::m_Shaders[stringID];
	}

	Texture* Crescent::Resources::LoadTexture(const std::string& name, const std::string& filePath, GLenum textureTarget, GLenum textureFormat, bool srgb)
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
}