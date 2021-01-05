#pragma once
#include <GL/glew.h>

namespace CrescentEngine
{
	class Texture2D
	{
	public:
		Texture2D() {}
		void LoadTexture(const std::string& filePath);
		void BindTexture();
		void UnbindTexture();

	public:
		GLenum m_TextureTarget = GL_TEXTURE_2D;           // what type of texture we're dealing with

	private:
		unsigned int m_TextureID = 0;
	};
}