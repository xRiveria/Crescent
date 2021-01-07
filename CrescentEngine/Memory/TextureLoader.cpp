#include "CrescentPCH.h"
#include "TextureLoader.h"
#include "../Shading/Texture.h"
#include <stb_image/stb_image.h>

namespace Crescent
{
	Texture TextureLoader::LoadTexture(const std::string& filePath, GLenum textureTarget, GLenum textureInternalFormat, bool sRGB)
	{
		Texture texture;
		texture.m_TextureTarget = textureTarget;
		texture.m_TextureInternalFormat = textureInternalFormat;
		
		if (texture.m_TextureInternalFormat == GL_RGB || texture.m_TextureInternalFormat == GL_SRGB)
		{
			texture.m_TextureInternalFormat = sRGB ? GL_SRGB : GL_RGB;
		}
		if (texture.m_TextureInternalFormat == GL_RGBA || texture.m_TextureInternalFormat == GL_SRGB_ALPHA)
		{
			texture.m_TextureInternalFormat = sRGB ? GL_SRGB_ALPHA : GL_RGBA;
		}

		//Flip textures on their Y coordinates while loading.
		stbi_set_flip_vertically_on_load(true);

		int textureWidth, textureHeight, componentCount;
		unsigned char* textureData = stbi_load(filePath.c_str(), &textureWidth, &textureHeight, &componentCount, 0);
		if (textureData)
		{
			GLenum format;
			if (componentCount == 1)
			{
				format = GL_RED;
			}
			else if (componentCount == 3)
			{
				format = GL_RGB;
			}
			else if (componentCount == 4)
			{
				format = GL_RGBA;
			}

			if (textureTarget == GL_TEXTURE_1D)
			{
				texture.GenerateTexture(textureWidth, texture.m_TextureInternalFormat, format, GL_UNSIGNED_BYTE, textureData);
			}
			else if (textureTarget == GL_TEXTURE_2D)
			{
				texture.GenerateTexture(textureWidth, textureHeight, texture.m_TextureInternalFormat, format, GL_UNSIGNED_BYTE, textureData);
			}
			stbi_image_free(textureData);
		}
		else
		{
			CrescentError("Texture failed to load at path " + filePath);
			stbi_image_free(textureData);
			return texture;
		}
	}

	Texture TextureLoader::LoadHDRTexture(const std::string& filePath)
	{
		return Texture();
	}
}