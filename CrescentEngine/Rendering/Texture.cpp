#include "CrescentPCH.h"
#include "Texture.h"
#include <stb_image/stb_image.h>

namespace CrescentEngine
{
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

	void Texture2D::BindTexture()
	{
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, m_TextureID);
	}

	void Texture2D::UnbindTexture()
	{
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
