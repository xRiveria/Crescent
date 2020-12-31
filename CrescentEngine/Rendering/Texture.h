#pragma once
#include <GL/glew.h>

namespace CrescentEngine
{
	class Texture2D
	{
	public:
		Texture2D() {}

		//1D Texture Creation
		void GenerateTexture(unsigned int textureWidth, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData);

		//2D Texture Creation
		void GenerateTexture(unsigned int textureWidth, unsigned int textureHeight, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData);

		//3D texture Creation
		void GenerateTexture(unsigned int textureWidth, unsigned int textureHeight, unsigned int textureDepth, GLenum textureInternalFormat, GLenum textureFormat, GLenum textureDataType, void* textureData);

		void LoadTexture(const std::string& filePath);
		void BindTexture();
		void UnbindTexture();

	public:
		GLenum m_TextureTarget = GL_TEXTURE_2D;					 //The type of texture we are dealing with.
		GLenum m_TextureInternalFormat = GL_RGBA;				 //The number of color components.
		GLenum m_Format = GL_RGBA;								 //The format each texel is stored in.
		GLenum m_DataType = GL_UNSIGNED_BYTE;
		GLenum m_MinificationFilter = GL_LINEAR_MIPMAP_LINEAR;	 //What filter method to use during minification.
		GLenum m_MagnificationFilter = GL_LINEAR;				 //What filter method to use during magnification.
		GLenum m_WrapS = GL_REPEAT;								 //Wrapping method of the S (X) coordinate.
		GLenum m_WrapT = GL_REPEAT;								 //Wrapping method of the T (Y) coordinate.
		GLenum m_WrapR = GL_REPEAT;								 //Wrapping method of the R (Z) coordinate.
		bool m_MipmappingEnabled = true;		

		unsigned int m_TextureWidth = 0;
		unsigned int m_TextureHeight = 0;
		unsigned int m_TextureDepth = 0;

	private:
		unsigned int m_TextureID = 0;
	};
}