#pragma once
#include <GL/glew.h>

namespace Crescent
{
	class Texture
	{
	public:
		Texture();
		~Texture();

		//1D Texture Generation
		void GenerateTexture(unsigned int textureWidth, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData);
		//2D Texture Generation
		void GenerateTexture(unsigned int textureWidth, unsigned int textureHeight, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData);
		//3D Texture Generation
		void GenerateTexture(unsigned int textureWidth, unsigned int textureHeight, unsigned int textureDepth, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData);
		//Resizes the textures, adding new (empty) texture memory in the process.
		void ResizeTexture(unsigned int textureWidth, unsigned int textureHeight = 0, unsigned int textureDepth = 0);

		//Update Relevant Texture State
		void SetWrappingMode(GLenum wrappingMode, bool binding = false);
		void SetMinificationFilter(GLenum minificationFilter, bool binding = false);
		void SetMagnificationFilter(GLenum magnificationFilter, bool binding = false);

		//Retrieves
		unsigned int RetrieveTextureID() const;

		void BindTexture(int textureUnit = -1);
		void UnbindTexture();

	public:
		GLenum m_TextureTarget = GL_TEXTURE_2D;           // what type of texture we're dealing with
		GLenum m_TextureInternalFormat = GL_RGBA;
		GLenum m_TextureFormat = GL_RGBA;
		GLenum m_TextureDataType = GL_UNSIGNED_BYTE; 

		GLenum m_TextureMinificationFilter = GL_LINEAR_MIPMAP_LINEAR;
		GLenum m_TextureMagnificationFilter = GL_LINEAR;

		GLenum m_TextureWrapS = GL_REPEAT;
		GLenum m_TextureWrapT = GL_REPEAT;
		GLenum m_TextureWrapR = GL_REPEAT;

		bool m_MipmappingEnabled = true;

		unsigned int m_TextureWidth = 0;
		unsigned int m_TextureHeight = 0;
		unsigned int m_TextureDepth = 0;

	private:
		unsigned int m_TextureID = 0;
	};
}