#pragma once
#include <GL/glew.h>

namespace CrescentEngine
{
	class TextureCube
	{
	public:
		TextureCube();
		~TextureCube();

		//Default initialize all cubemap faces with default values.
		void DefaultInitializeCubemapTexture(unsigned int textureWidth, unsigned int textureHeight, GLenum textureFormat, GLenum textureDataType, bool mipmappingEnabled = false);

		//Cubemap texture generation per face.
		void GenerateCubeFace(GLenum cubeFace, unsigned int faceTextureWidth, unsigned int faceTextureHeight, GLenum textureFormat, GLenum textureDataType, unsigned char* textureData);
		
		//Mipmapping
		void SetTextureMipmap(GLenum cubeFace, unsigned int textureWidth, unsigned int textureHeight, GLenum textureFormat, GLenum textureDataType, unsigned int mipmapLevel, unsigned char* textureData);

		//Resize (note that its values will be uninitialized).
		void ResizeCubemap(unsigned int faceTextureWidth, unsigned int faceTextureHeight);

		void BindCubemapTexture(int textureUnit = -1);
		void UnbindCubemapTexture();

	public:
		unsigned int m_CubemapTextureID;

		//Texture specific information for each face. Create getters/setters, like for everything else.
		GLenum m_TextureInternalFormat = GL_RGBA;				 //The number of color components.
		GLenum m_Format = GL_RGBA;								 //The format each texel is stored in.
		GLenum m_DataType = GL_UNSIGNED_BYTE;
		GLenum m_MinificationFilter = GL_LINEAR;				 //What filter method to use during minification.
		GLenum m_MagnificationFilter = GL_LINEAR;				 //What filter method to use during magnification.
		GLenum m_WrapS = GL_REPEAT;								 //Wrapping method of the S (X) coordinate.
		GLenum m_WrapT = GL_REPEAT;								 //Wrapping method of the T (Y) coordinate.
		GLenum m_WrapR = GL_REPEAT;								 //Wrapping method of the R (Z) coordinate.
		bool m_MipmappingEnabled = false;

		unsigned int m_FaceTextureWidth = 0;
		unsigned int m_FaceTextureHeight = 0;
	};
}