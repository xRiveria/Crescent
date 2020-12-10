#pragma once
#include "OpenGLRenderer.h"

class Texture
{
public:
	Texture(const std::string& path);
	~Texture();

	void Bind(unsigned int slot = 0) const;  //Allows us to specify a slot we want to bind the texture to. In OpenGl, we have these slots because we have the ability to bind more than one texture at once. In OpenGl, there are slots for us to bind textures to. On Windows, we typically have 32 texture slots. Of course, we can query OpenGL for many we have. 
	void Unbind() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }

private:
	unsigned int m_RendererID;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;  //Local Storage for the Texture
	int m_Width, m_Height, m_BPP; //Bits per pixel.
};

//The number of bits of information stored per pixel of an image or displayed by a graphics adapter. The more bits there are, the more colours can be represented,
//but the more memory is required to store or display the image.