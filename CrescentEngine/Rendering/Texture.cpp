#include "CrescentPCH.h"
#include "Texture.h"
#include <stb_image/stb_image.h>

namespace Crescent
{
	Texture2D::Texture2D()
	{

	}

	Texture2D::~Texture2D()
	{

	}

	void Texture2D::GenerateTexture(unsigned int textureWidth, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData)
	{
		glGenTextures(1, &m_TextureID);

		m_TextureWidth = textureWidth;
		m_TextureHeight = 0;
		m_TextureDepth = 0;
		m_TextureInternalFormat = textureInternalFormat;
		m_Format = textureFormat;
		m_DataType = textureDataType;

		if (m_TextureTarget != GL_TEXTURE_1D)
		{
			CrescentError("Requested genertion of 1D texture, but found another target type.");
		}

		BindTexture();
		glTexImage1D(m_TextureTarget, 0, textureInternalFormat, textureWidth, 0, textureFormat, textureDataType, textureData);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MIN_FILTER, m_MinificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MAG_FILTER, m_MagnificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, m_WrapS);

		if (m_MipmappingEnabled)
		{
			glGenerateMipmap(m_TextureTarget);
		}

		UnbindTexture();
	}

	void Texture2D::GenerateTexture(unsigned int textureWidth, unsigned int textureHeight, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData)
	{
		glGenTextures(1, &m_TextureID);

		m_TextureWidth = textureWidth;
		m_TextureHeight = textureHeight;
		m_TextureDepth = 0;
		m_TextureInternalFormat = textureInternalFormat;
		m_Format = textureFormat;
		m_DataType = textureDataType;

		if (m_TextureTarget != GL_TEXTURE_2D)
		{
			CrescentError("Requested genertion of 2D texture, but found another target type.");
		}

		BindTexture();
		glTexImage2D(m_TextureTarget, 0, textureInternalFormat, textureWidth, textureHeight, 0, textureFormat, textureDataType, textureData);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MIN_FILTER, m_MinificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MAG_FILTER, m_MagnificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, m_WrapS);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_T, m_WrapT);

		if (m_MipmappingEnabled)
		{
			glGenerateMipmap(m_TextureTarget);
		}

		UnbindTexture();
	}

	void Texture2D::GenerateTexture(unsigned int textureWidth, unsigned int textureHeight, unsigned int textureDepth, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData)
	{
		glGenTextures(1, &m_TextureID);

		m_TextureWidth = textureWidth;
		m_TextureHeight = textureHeight;
		m_TextureDepth = textureDepth;
		m_TextureInternalFormat = textureInternalFormat;
		m_Format = textureFormat;
		m_DataType = textureDataType;

		if (m_TextureTarget != GL_TEXTURE_3D)
		{
			CrescentError("Requested genertion of 3D texture, but found another target type.");
		}

		BindTexture();
		glTexImage3D(m_TextureTarget, 0, textureInternalFormat, textureWidth, textureHeight, textureDepth, 0, textureFormat, textureDataType, textureData);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MIN_FILTER, m_MinificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MAG_FILTER, m_MagnificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, m_WrapS);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_T, m_WrapT);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_R, m_WrapR);

		if (m_MipmappingEnabled)
		{
			glGenerateMipmap(m_TextureTarget);
		}

		UnbindTexture();
	}

	void Texture2D::ResizeTexture(unsigned int newTextureWidth, unsigned int newTextureHeight, unsigned int textureDepth)
	{
		BindTexture();

		if (m_TextureTarget == GL_TEXTURE_1D)
		{
			glTexImage1D(GL_TEXTURE_1D, 0, m_TextureInternalFormat, newTextureWidth, 0, m_Format, m_DataType, 0);
		}
		else if (m_TextureTarget == GL_TEXTURE_2D)
		{
			if (newTextureHeight < 0)
			{
				CrescentError("Texture has a height of less than 0.");
			}
			glTexImage2D(GL_TEXTURE_2D, 0, m_TextureInternalFormat, newTextureWidth, newTextureHeight, 0, m_Format, m_DataType, 0);
		}
		else if (m_TextureTarget == GL_TEXTURE_3D)
		{
			if (newTextureHeight < 0 && textureDepth < 0)
			{
				CrescentError("Texture has a height and depth of 0.");
			}
			glTexImage3D(GL_TEXTURE_3D, 0, m_TextureInternalFormat, newTextureWidth, newTextureHeight, textureDepth, 0, m_Format, m_DataType, 0);
		}
	}

	void Texture2D::SetMinificationFilter(GLenum minificationFilter, bool bindTexture)
	{
		if (bindTexture)
		{
			BindTexture();
		}

		glTexParameteri(m_TextureTarget, GL_TEXTURE_MIN_FILTER, minificationFilter);
	}

	void Texture2D::SetMagnificationFilter(GLenum magnificationFilter, bool bindTexture)
	{
		if (bindTexture)
		{
			BindTexture();
		}

		glTexParameteri(m_TextureTarget, GL_TEXTURE_MAG_FILTER, magnificationFilter);
	}

	void Texture2D::SetWrappingMode(GLenum textureWrappingMode, bool bindTexture)
	{
		if (bindTexture)
		{
			BindTexture();
		}

		if (m_TextureTarget == GL_TEXTURE_1D)
		{
			m_WrapS = textureWrappingMode;
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, textureWrappingMode);
		}
		else if (m_TextureTarget == GL_TEXTURE_2D)
		{
			m_WrapS = textureWrappingMode;
			m_WrapT = textureWrappingMode;

			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, textureWrappingMode);
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_T, textureWrappingMode);
		}
		else if (m_TextureTarget == GL_TEXTURE_3D)
		{
			m_WrapS = textureWrappingMode;
			m_WrapT = textureWrappingMode;
			m_WrapR = textureWrappingMode;

			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, textureWrappingMode);
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_T, textureWrappingMode);
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_R, textureWrappingMode);
		}
	}

	void Texture2D::BindTexture(int textureUnit)
	{
		if (textureUnit >= 0)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);
		}
		glBindTexture(GL_TEXTURE_2D, m_TextureID);
	}

	void Texture2D::UnbindTexture()
	{
		glBindTexture(m_TextureTarget, 0);
	}

	//=========================================================================

	void Texture2D::LoadTexture(const std::string& filePath)
	{
		glGenTextures(1, &m_TextureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, m_TextureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			CrescentInfo("Failed to load Texture Image.");
			stbi_image_free(data);
		}
	}
}
