#pragma once
#include <GL/glew.h>
#include <string>

namespace Crescent
{
	class Texture;
	class TextureCube;

	/*
		Manages all custom logic for loading a variety of different texture files.
	*/

	class TextureLoader
	{
	public:
		static Texture LoadTexture(const std::string& filePath, GLenum textureTarget, GLenum textureInternalFormat, bool sRGB = false);
		static Texture LoadHDRTexture(const std::string& filePath);
		//Follows OpenGL's cubemap enums.
		static TextureCube LoadTextureCube(const std::string& right, const std::string& left, const std::string& top, const std::string& bottom, const std::string& front, const std::string& back);
		static TextureCube LoadTextureCube(const std::string& folderPath); 	//Assumes default names for cubemap faces.
	};
}
