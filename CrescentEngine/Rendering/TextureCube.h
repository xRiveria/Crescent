#pragma once
#include <GL/glew.h>

namespace CrescentEngine
{
	class TextureCube
	{
	public:
		//Cubemap Texture Generation Per Face.
		void GenerateCubeTextures(GLenum textureFace, unsigned int textureWidth, unsigned int textureHeight, GLenum textureFormat, GLenum textureDataType, unsigned char* textureData);
	
	public:
		bool m_MipmappingEnabled = false;
	};
}