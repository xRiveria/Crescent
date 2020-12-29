#include "CrescentPCH.h"
#include "TextureLoader.h"
#include <stb_image/stb_image.h>

namespace CrescentEngine
{
	Texture2D TextureLoader::LoadTexture(const std::string& filePath, GLenum textureTarget, GLenum textureInternalFormat, bool srgb)
	{
		Texture2D texture;
		texture.m_TextureTarget = textureTarget;
		texture.m_TextureInternalFormat = textureInternalFormat;

		if (texture.m_TextureInternalFormat == GL_RGB || texture.m_TextureInternalFormat == GL_SRGB)
		{
			texture.m_TextureInternalFormat = srgb ? GL_SRGB : GL_RGB;
		}

		if (texture.m_TextureInternalFormat == GL_RGBA || texture.m_TextureInternalFormat == GL_SRGB_ALPHA)
		{
			texture.m_TextureInternalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
		}

		//Flip textures on their Y coordinate while loading.
		stbi_set_flip_vertically_on_load(true);

		int textureWidth, textureHeight, componentCount;
		unsigned char* textureData = stbi_load(filePath.c_str(), &textureWidth, &textureHeight, &componentCount, 0);

		if (textureData)
		{
			GLenum textureFormat;
			switch (componentCount)
			{
				case 1:
					textureFormat = GL_RED;
					break;
				case 3:
					textureFormat = GL_RGB;
					break;
				case 4:
					textureFormat = GL_RGBA;
					break;
			}

			if (textureTarget == GL_TEXTURE_1D)
			{
				texture.GenerateTexture(textureWidth, texture.m_TextureInternalFormat, textureFormat, GL_UNSIGNED_BYTE, textureData);
			}
			else if (textureTarget == GL_TEXTURE_2D)
			{
				texture.GenerateTexture(textureWidth, textureHeight, texture.m_TextureInternalFormat, textureFormat, GL_UNSIGNED_BYTE, textureData);
			}

			stbi_image_free(textureData);
		}
		else
		{
			CrescentInfo("Texture failed to load at path: " + filePath);
			stbi_image_free(textureData);
			return texture;
		}

		texture.m_TextureWidth = textureWidth;
		texture.m_TextureHeight = textureHeight;

		return Texture2D();
	}
}