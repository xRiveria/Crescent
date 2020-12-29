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

	Texture2D TextureLoader::LoadHDRTexture(const std::string& filePath)
	{
		Texture2D texture;
		texture.m_TextureTarget = GL_TEXTURE_2D;
		texture.m_MinificationFilter = GL_LINEAR;
		texture.m_MipmappingEnabled = false;

		stbi_set_flip_vertically_on_load(true);

		if (stbi_is_hdr(filePath.c_str()))
		{
			int textureWidth, textureHeight, componentCount;

			float* textureData = stbi_loadf(filePath.c_str(), &textureWidth, &textureHeight, &componentCount, 0);
			if (textureData)
			{
				GLenum internalFormat, format;
				if (componentCount == 3)
				{
					internalFormat = GL_RGB32F;
					format = GL_RGB;
				}
				else if (componentCount == 4)
				{
					internalFormat = GL_RGBA32F;
					format = GL_RGBA;
				}

				texture.GenerateTexture(textureWidth, textureHeight, internalFormat, format, GL_FLOAT, textureData);
				stbi_image_free(textureData);
			}

			texture.m_TextureWidth = textureWidth;
			texture.m_TextureHeight = textureHeight;
		}
		else
		{
			CrescentInfo("Trying to load a HDR texture with invalid path or texture is not HDR: " + filePath + ".");
		}

		return texture;
	}

	TextureCube TextureLoader::LoadTextureCube(const std::string& topTexturePath, const std::string& bottomTexturePath, const std::string& leftTexturePath, const std::string& rightTexturePath, const std::string& frontTexturePath, const std::string& backTexturePath)
	{
		TextureCube texture;

		//Disable Y Flip on Cubemaps.
		stbi_set_flip_vertically_on_load(false);

		std::vector<std::string> cubeFaces = { topTexturePath, bottomTexturePath, leftTexturePath, rightTexturePath, frontTexturePath, backTexturePath };
		for (unsigned int i = 0; i < cubeFaces.size(); ++i)
		{
			int textureWidth, textureHeight, componentCount;
			unsigned char* textureData = stbi_load(cubeFaces[i].c_str(), &textureWidth, &textureHeight, &componentCount, 0);

			if (textureData)
			{
				GLenum textureFormat;
				if (textureFormat == 3)
				{
					textureFormat = GL_RGB;
				}
				else if (textureFormat == 4)
				{
					textureFormat = GL_RGBA;
				}

				texture.GenerateCubeTextures(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, textureWidth, textureHeight, textureFormat, GL_UNSIGNED_INT, textureData);
				stbi_image_free(textureData);
			}
			else
			{
				CrescentInfo("Cube texture at path: " + cubeFaces[i] + " failed to load.");
				stbi_image_free(textureData);
				return texture;
			}
		}

		if (texture.m_MipmappingEnabled)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

		return texture;
	}

	TextureCube TextureLoader::LoadTextureCube(const std::string& texturesFolderPath)
	{
		return TextureLoader::LoadTextureCube(texturesFolderPath + "top.jpg", texturesFolderPath + "bottom.jpg", texturesFolderPath + "left.jpg", 
			texturesFolderPath + "right.jpg", texturesFolderPath + "front.jpg", texturesFolderPath + "back.jpg");
	}
}