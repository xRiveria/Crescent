#include "CrescentPCH.h"
#include "TextureCube.h"

namespace Crescent
{
	TextureCube::TextureCube()
	{

	}

	TextureCube::~TextureCube()
	{

	}

	void TextureCube::DefaultInitialize(unsigned int cubeFaceWidth, unsigned int cubeFaceHeight, GLenum textureCubeFormat, GLenum textureCubeDataType, bool mipmappingEnabled)
	{
		glGenTextures(1, &m_TextureCubeID);

		m_TextureCubeFaceWidth = cubeFaceWidth;
		m_TextureCubeFaceHeight = cubeFaceHeight;
		m_TextureCubeFormat = textureCubeFormat;
		m_TextureCubeDataType = textureCubeDataType;
		m_MipmappingEnabled = mipmappingEnabled;

		if (textureCubeDataType == GL_HALF_FLOAT && textureCubeFormat == GL_RGB)
		{
			m_TextureCubeInternalFormat = GL_RGB16F;
		}
		else if (textureCubeDataType == GL_FLOAT && textureCubeFormat == GL_RGB)
		{
			m_TextureCubeInternalFormat = GL_RGB32F;
		}
		else if (textureCubeDataType == GL_HALF_FLOAT && textureCubeFormat == GL_RGBA)
		{
			m_TextureCubeInternalFormat = GL_RGBA16F;
		}
		else if (textureCubeDataType == GL_FLOAT && textureCubeFormat == GL_RGBA)
		{
			m_TextureCubeInternalFormat = GL_RGBA32F;
		}

		BindTextureCube();
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_TextureCubeMinificationFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_TextureCubeMagnificationFilter);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, m_TextureCubeWrapS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, m_TextureCubeWrapT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, m_TextureCubeWrapR);

		for (unsigned int i = 0; i < 6; i++) //6 faces for a cubemap.
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_TextureCubeInternalFormat, cubeFaceWidth, cubeFaceHeight, 0, textureCubeFormat, textureCubeDataType, nullptr);
		}

		if (mipmappingEnabled)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}
	}

	void TextureCube::GenerateCubemapFace(GLenum cubeFace, unsigned int cubeFaceWidth, unsigned int cubeFaceHeight, GLenum cubeFaceFormat, GLenum cubeFaceDataType, unsigned char* cubeFaceData)
	{
		if (cubeFaceWidth == 0)
		{
			glGenTextures(1, &m_TextureCubeID);
		}

		m_TextureCubeFaceWidth = cubeFaceWidth;
		m_TextureCubeFaceHeight = cubeFaceHeight;
		m_TextureCubeFormat = cubeFaceFormat;
		m_TextureCubeDataType = cubeFaceDataType;

		BindTextureCube();

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_TextureCubeMinificationFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_TextureCubeMagnificationFilter);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, m_TextureCubeWrapS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, m_TextureCubeWrapT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, m_TextureCubeWrapR);

		glTexImage2D(cubeFace, 0, cubeFaceFormat, cubeFaceWidth, cubeFaceHeight, 0, cubeFaceFormat, cubeFaceDataType, cubeFaceData);
	}

	void TextureCube::SetMipmapFace(GLenum cubeFace, unsigned int cubeFaceWidth, unsigned int cubeFaceHeight, GLenum cubeFaceFormat, GLenum cubeFaceDataType, unsigned int cubeFaceMipmapLevel, unsigned char* cubeFaceData)
	{
		BindTextureCube();
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, cubeFaceMipmapLevel, 0, 0, cubeFaceWidth, cubeFaceHeight, cubeFaceFormat, cubeFaceDataType, cubeFaceData);
	}

	void TextureCube::ResizeTextureCube(unsigned int newWidth, unsigned int newHeight)
	{
		m_TextureCubeFaceWidth = newWidth;
		m_TextureCubeFaceHeight = newHeight;

		BindTextureCube();
		for (unsigned int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_TextureCubeFormat, newWidth, newHeight, 0, m_TextureCubeFormat, m_TextureCubeDataType, nullptr);
		}
	}

	void TextureCube::BindTextureCube(int textureUnit)
	{
		if (textureUnit >= 0)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureCubeID);
	}

	void TextureCube::UnbindTextureCube()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}