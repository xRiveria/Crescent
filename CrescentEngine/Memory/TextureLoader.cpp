#include "CrescentPCH.h"
#include "TextureLoader.h"
#include "../Shading/Texture.h"
#include "../Shading/TextureCube.h"
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

		texture.m_TextureWidth = textureWidth;
		texture.m_TextureHeight = textureHeight;

		return texture;
	}

	Texture TextureLoader::LoadHDRTexture(const std::string& filePath)
	{
		Texture texture;
		texture.m_TextureTarget = GL_TEXTURE_2D;
		texture.m_TextureMinificationFilter = GL_LINEAR;
		texture.m_MipmappingEnabled = false;

		stbi_set_flip_vertically_on_load(true);

		if (stbi_is_hdr(filePath.c_str()))
		{
			int textureWidth, textureHeight, componentCount;
			float* textureData = stbi_loadf(filePath.c_str(), &textureWidth, &textureHeight, &componentCount, 0); //Automatically maps the HDR values to a list of floating point values: 32 bits per channe and 3 channels per color.
			
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
			CrescentError("Trying to load a HDR texture that has an invalid path or is not HDR: " + filePath + ".");
		}

		return texture;
	}

	TextureCube TextureLoader::LoadTextureCube(const std::string& top, const std::string& bottom, const std::string& left, const std::string& right, const std::string& front, const std::string& back)
	{
		TextureCube textureCube;

		//Disable Y flip on Cubemaps.
		stbi_set_flip_vertically_on_load(false);

		std::vector<std::string> faces = { top, bottom, left, right, front, back };
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			int textureWidth, textureHeight, componentCount;
			unsigned char* textureData = stbi_load(faces[i].c_str(), &textureWidth, &textureHeight, &componentCount, 0);

			if (textureData)
			{
				GLenum format;
				if (componentCount == 3)
				{
					format = GL_RGB;
				}
				else
				{
					format = GL_RGBA;
				}

				textureCube.GenerateCubemapFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, textureWidth, textureHeight, format, GL_UNSIGNED_BYTE, textureData);
				stbi_image_free(textureData);
			}
			else
			{
				CrescentInfo("Cube Texture at Path: " + faces[i] + " failed to load.");
				stbi_image_free(textureData);
				return textureCube;
			}
		}
		if (textureCube.m_MipmappingEnabled)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

		return textureCube;
	}

	TextureCube TextureLoader::LoadTextureCube(const std::string& folderPath)
	{
		return LoadTextureCube(folderPath + "right.jpg", folderPath + "left.jpg", folderPath + "top.jpg", folderPath + "bottom.jpg", folderPath + "front.jpg", folderPath + "back.jpg");
	}
}