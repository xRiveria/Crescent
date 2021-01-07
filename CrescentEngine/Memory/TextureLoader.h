#pragma once
#include <GL/glew.h>
#include <string>

namespace Crescent
{
	class Texture;

	/*
		Manages all custom logic for loading a variety of different texture files.
	*/

	class TextureLoader
	{
	public:
		static Texture LoadTexture(const std::string& filePath, GLenum textureTarget, GLenum textureInternalFormat, bool sRGB = false);
		static Texture LoadHDRTexture(const std::string& filePath);
	};
}
