#pragma once
#include "../Rendering/Renderer.h"
#include "../Rendering/Shader.h"
#include "../Rendering/Texture.h"
#include "../Rendering/TextureCube.h"
#include "Scene/SceneNode.h"
#include <map>
#include <vector>

namespace Crescent
{
	/*
		Global resource manager. This class manages and maintains all resource memory used throughout the rendering application.
		New resources are loaded from here, and duplicate resouce loads are prevented. Every resource is referenced by a hashed string ID.
	*/

	class Resources
	{
	public:
		static void InitializeResourceManager();
		static void Clean();

		//Shader Resources
		static Shader* LoadShader(const std::string& name, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, std::vector<std::string> defines = std::vector<std::string>());
		static Shader* RetrieveShader(const std::string& name);

		//Textures
		static Texture2D* LoadTexture(const std::string& name, const std::string& filePath, GLenum textureTarget = GL_TEXTURE_2D, GLenum textureInternalFormat = GL_RGBA, bool srgb = false);
		static Texture2D* LoadHDRTexture(const std::string& name, const std::string& filePath);
		static Texture2D* RetrieveTexture(const std::string& name);
		
		static TextureCube* LoadTextureCube(const std::string& name, const std::string& texturesFolderPath);
		static TextureCube* RetrieveTextureCube(const std::string name);

		//Mesh/Scene Resources
		static SceneNode* LoadMesh(Renderer* renderer, const std::string& name, const std::string& filePath);
		static SceneNode* RetrieveMesh(const std::string& name);

	private:
		//Disallow creation of any Resources object. This is a static object.
		Resources();

	private:
		//We index all resources with a hashed string ID.
		static std::map<unsigned int, Shader> m_Shaders;
		static std::map<unsigned int, Texture2D> m_Textures;
		static std::map<unsigned int, TextureCube> m_TextureCubes;
		static std::map<unsigned int, SceneNode*> m_Meshes;
	};
}