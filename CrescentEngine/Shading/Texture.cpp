#include "CrescentPCH.h"
#include "Texture.h"
#include <stb_image/stb_image.h>

namespace Crescent
{
	Texture::Texture()
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::GenerateTexture(unsigned int textureWidth, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData)
	{
		glGenTextures(1, &m_TextureID);

		m_TextureWidth = textureWidth;
		m_TextureHeight = 0;
		m_TextureDepth = 0;
		m_TextureInternalFormat = textureInternalFormat;
		m_TextureFormat = textureFormat;
		m_TextureDataType = textureDataType;

		if (m_TextureTarget != GL_TEXTURE_1D)
		{
			CrescentError("Error! Wrong function used to load texture.");
		}
		BindTexture();

		glTexImage1D(m_TextureTarget, 0, textureInternalFormat, textureWidth, 0, textureFormat, textureDataType, textureData);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MIN_FILTER, m_TextureMinificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MAG_FILTER, m_TextureMagnificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, m_TextureWrapS);
		if (m_MipmappingEnabled)
		{
			glGenerateMipmap(m_TextureTarget);
		}

		UnbindTexture();
	}

	void Texture::GenerateTexture(unsigned int textureWidth, unsigned int textureHeight, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData)
	{
		glGenTextures(1, &m_TextureID);

		m_TextureWidth = textureWidth;
		m_TextureHeight = textureHeight;
		m_TextureDepth = 0;
		m_TextureInternalFormat = textureInternalFormat;
		m_TextureFormat = textureFormat;
		m_TextureDataType = textureDataType;

		if (m_TextureTarget != GL_TEXTURE_2D)
		{
			CrescentError("Error! Wrong function used to load texture.");
		}
		BindTexture();

		glTexImage2D(m_TextureTarget, 0, textureInternalFormat, textureWidth, textureHeight, 0, textureFormat, textureDataType, textureData);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MIN_FILTER, m_TextureMinificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MAG_FILTER, m_TextureMagnificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, m_TextureWrapS);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_T, m_TextureWrapT);

		if (m_MipmappingEnabled)
		{
			glGenerateMipmap(m_TextureTarget);
		}

		UnbindTexture();
	}

	void Texture::GenerateTexture(unsigned int textureWidth, unsigned int textureHeight, unsigned int textureDepth, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData)
	{
		glGenTextures(1, &m_TextureID);

		m_TextureWidth = textureWidth;
		m_TextureHeight = textureHeight;
		m_TextureDepth = textureDepth;
		m_TextureInternalFormat = textureInternalFormat;
		m_TextureFormat = textureFormat;
		m_TextureDataType = textureDataType;

		if (m_TextureTarget != GL_TEXTURE_3D)
		{
			CrescentError("Error! Wrong function used to load texture.");
		}
		BindTexture();

		glTexImage3D(m_TextureTarget, 0, textureInternalFormat, textureWidth, textureHeight, textureDepth, 0, textureFormat, textureDataType, textureData);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MIN_FILTER, m_TextureMinificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MAG_FILTER, m_TextureMagnificationFilter);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, m_TextureWrapS);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_T, m_TextureWrapT);
		glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_R, m_TextureWrapR);

		if (m_MipmappingEnabled)
		{
			glGenerateMipmap(m_TextureTarget);
		}

		UnbindTexture();
	}

	void Texture::ResizeTexture(unsigned int textureWidth, unsigned int textureHeight, unsigned int textureDepth)
	{
		BindTexture();
		if (m_TextureTarget == GL_TEXTURE_1D)
		{
			glTexImage1D(GL_TEXTURE_1D, 0, m_TextureInternalFormat, textureWidth, 0, m_TextureFormat, m_TextureDataType, 0);
		}
		else if (m_TextureTarget == GL_TEXTURE_2D)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, m_TextureInternalFormat, textureWidth, textureHeight, 0, m_TextureFormat, m_TextureDataType, 0);
		}
		else if (m_TextureTarget == GL_TEXTURE_3D)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, m_TextureInternalFormat, textureWidth, textureHeight, textureDepth, 0, m_TextureFormat, m_TextureDataType, 0);
		}
	}

	void Texture::SetWrappingMode(GLenum wrappingMode, bool binding)
	{
		if (binding)
		{
			BindTexture();
		}
		if (m_TextureTarget == GL_TEXTURE_1D)
		{
			m_TextureWrapS = wrappingMode;
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, wrappingMode);
		}
		else if (m_TextureTarget == GL_TEXTURE_2D)
		{
			m_TextureWrapS = wrappingMode;
			m_TextureWrapT = wrappingMode;
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, wrappingMode);
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_T, wrappingMode);
		}
		else if (m_TextureTarget == GL_TEXTURE_3D)
		{
			m_TextureWrapS = wrappingMode;
			m_TextureWrapT = wrappingMode;
			m_TextureWrapR = wrappingMode;
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, wrappingMode);
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_T, wrappingMode);
			glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_R, wrappingMode);
		}
	}

	void Texture::SetMinificationFilter(GLenum minificationFilter, bool binding)
	{
		if (binding)
		{
			BindTexture();
		}
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MIN_FILTER, minificationFilter);
	}

	void Texture::SetMagnificationFilter(GLenum magnificationFilter, bool binding)
	{
		if (binding)
		{
			BindTexture();
		}
		glTexParameteri(m_TextureTarget, GL_TEXTURE_MAG_FILTER, magnificationFilter);
	}

	unsigned int Texture::RetrieveTextureID() const
	{
		return m_TextureID;
	}

	void Texture::BindTexture(int textureUnit)
	{
		if (textureUnit >= 0)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);
		}
		glBindTexture(m_TextureTarget, m_TextureID);
	}

	void Texture::UnbindTexture()
	{
		glBindTexture(m_TextureTarget, 0);
	}
}
