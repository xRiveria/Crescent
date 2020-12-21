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

	private:
		unsigned int m_TextureID = 0;
	};
}