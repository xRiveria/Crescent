#pragma once
#include <GL/glew.h>
#include <string>
#include "../Rendering/Texture.h"

namespace CrescentEngine
{
	/*
		Manages all custom logic for loading a variety of different texture files.
	*/

	class TextureLoader
	{
	public:
		static Texture2D LoadTexture(const std::string& filePath, GLenum textureTarget, GLenum textureInternalFormat, bool srgb = false);
		static Texture2D LoadHDRTexture(const std::string& filePath);
	};
}

