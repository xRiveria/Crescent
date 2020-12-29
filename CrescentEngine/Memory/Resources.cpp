#include "CrescentPCH.h"
#include "Resources.h"
#include "../Utilities/StringID.h"
#include "ShaderLoader.h"

namespace CrescentEngine
{
	void Resources::InitializeResourceManager()
	{
		//Initialize default assets/resources that should always be avaliable, regardless of configuration.
		Texture2D placeholderTexture;
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

	Texture2D* CrescentEngine::Resources::LoadTexture(const std::string& name, const std::string& filePath, GLenum textureTarget, GLenum textureInternalFormat, bool srgb)
	{
		return nullptr;
	}
}