#pragma once
#include "Texture.h"
#include <GL/glew.h>
#include <vector>

namespace Crescent
{
	class TextureCube
	{
	public:
		TextureCube();
		~TextureCube();

		//Default initialize all cubemap faces with default values.
		void DefaultInitialize(unsigned int cubeFaceWidth, unsigned int cubeFaceHeight, GLenum textureCubeFormat, GLenum textureCubeDataType, bool mipmappingEnabled = false);
		//Cubemap texture generation per face.
		void GenerateCubemapFace(GLenum cubeFace, unsigned int cubeFaceWidth, unsigned int cubeFaceHeight, GLenum cubeFaceFormat, GLenum cubeFaceDataType, unsigned char* cubeFaceData);

		void SetMipmapFace(GLenum cubeFace, unsigned int cubeFaceWidth, unsigned int cubeFaceHeight, GLenum cubeFaceFormat, GLenum cubeFaceDataType, unsigned int cubeFaceMipmapLevel, unsigned char* cubeFaceData);

		//Resizing will uninitialize all values.
		void ResizeTextureCube(unsigned int newWidth, unsigned int newHeight);

		void BindTextureCube(int textureUnit = -1);
		void UnbindTextureCube();

	public:
		GLenum m_TextureCubeInternalFormat = GL_RGBA;		
		GLenum m_TextureCubeFormat = GL_RGBA;				
		GLenum m_TextureCubeDataType = GL_UNSIGNED_BYTE;
		
		GLenum m_TextureCubeMinificationFilter = GL_LINEAR;
		GLenum m_TextureCubeMagnificationFilter = GL_LINEAR;

		GLenum m_TextureCubeWrapS = GL_CLAMP_TO_EDGE;
		GLenum m_TextureCubeWrapT = GL_CLAMP_TO_EDGE;
		GLenum m_TextureCubeWrapR = GL_CLAMP_TO_EDGE;

		bool m_MipmappingEnabled = false;

		unsigned int m_TextureCubeFaceWidth = 0;
		unsigned int m_TextureCubeFaceHeight = 0;

		unsigned int m_TextureCubeID;
	};
}