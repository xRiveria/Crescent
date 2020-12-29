#pragma once
#include <GL/glew.h>
#include <string>
#include "../Rendering/Texture.h"
#include "../Rendering/TextureCube.h"

namespace CrescentEngine
{
	/*
		Manages all custom logic for loading a variety of different texture files.
	*/

	class TextureLoader
	{
	public:
		//Textures
		static Texture2D LoadTexture(const std::string& filePath, GLenum textureTarget, GLenum textureInternalFormat, bool srgb = false);
		static Texture2D LoadHDRTexture(const std::string& filePath);

		//Texture Cubemaps
		static TextureCube LoadTextureCube(const std::string& topTexturePath, const std::string& bottomTexturePath, const std::string& leftTexturePath, const std::string& rightTexturePath, const std::string& frontTexturePath, const std::string& backTexturePath);
		//Assumes default names for cubemap faces.
		static TextureCube LoadTextureCube(const std::string& texturesFolderPath);
	};
}

