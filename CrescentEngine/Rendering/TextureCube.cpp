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

	void TextureCube::DefaultInitializeCubemapTexture(unsigned int textureWidth, unsigned int textureHeight, GLenum textureFormat, GLenum textureDataType, bool mipmappingEnabled)
	{
		glGenTextures(1, &m_CubemapTextureID);

		m_FaceTextureWidth = textureWidth;
		m_FaceTextureHeight = textureHeight;
		m_Format = textureFormat;
		m_DataType = textureDataType;
		m_MipmappingEnabled = mipmappingEnabled;

		if (textureDataType == GL_HALF_FLOAT && textureFormat == GL_RGB)
		{
			m_TextureInternalFormat = GL_RGB16F;
		}
		else if (textureDataType == GL_FLOAT && textureFormat == GL_RGB)
		{
			m_TextureInternalFormat = GL_RGB32F;
		}
		else if (textureDataType == GL_HALF_FLOAT && textureFormat == GL_RGBA)
		{
			m_TextureInternalFormat = GL_RGBA16F;
		}
		else if (textureDataType == GL_FLOAT && textureFormat == GL_RGBA)
		{
			m_TextureInternalFormat = GL_RGBA32F;
		}

		BindCubemapTexture();
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_MinificationFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_MagnificationFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, m_WrapS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, m_WrapT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, m_WrapR);

		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_TextureInternalFormat, textureWidth, textureHeight, 0, textureFormat, textureDataType, nullptr);
		}

		if (m_MipmappingEnabled)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}
	}

	void TextureCube::GenerateCubeFace(GLenum cubeFace, unsigned int faceTextureWidth, unsigned int faceTextureHeight, GLenum textureFormat, GLenum textureDataType, unsigned char* textureData)
	{
		if (m_FaceTextureWidth == 0) //If there are currently no textures generated or a texture is invalid.
		{
			glGenTextures(1, &m_CubemapTextureID);
		}

		m_FaceTextureWidth = faceTextureWidth;
		m_FaceTextureHeight = faceTextureHeight;
		m_Format = textureFormat;
		m_DataType = textureDataType;

		BindCubemapTexture();

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_MinificationFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_MagnificationFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, m_WrapS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, m_WrapT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, m_WrapR);

		glTexImage2D(cubeFace, 0, textureFormat, faceTextureWidth, faceTextureHeight, 0, textureFormat, textureDataType, textureData);
	}

	void TextureCube::SetTextureMipmap(GLenum cubeFace, unsigned int textureWidth, unsigned int textureHeight, GLenum textureFormat, GLenum textureDataType, unsigned int mipmapLevel, unsigned char* textureData)
	{
		BindCubemapTexture();
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, mipmapLevel, 0, 0, textureWidth, textureHeight, textureFormat, textureDataType, textureData);
	}

	void TextureCube::ResizeCubemap(unsigned int faceTextureWidth, unsigned int faceTextureHeight)
	{
		m_FaceTextureWidth = faceTextureWidth;
		m_FaceTextureHeight = faceTextureHeight;

		BindCubemapTexture();

		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_Format, m_FaceTextureWidth, m_FaceTextureHeight, 0, m_Format, m_DataType, nullptr);
		}
	}

	void TextureCube::BindCubemapTexture(int textureUnit)
	{
		if (textureUnit >= 0)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTextureID);
	}

	void TextureCube::UnbindCubemapTexture()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}