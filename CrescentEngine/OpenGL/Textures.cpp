#include "CrescentPCH.h"
#include "Textures.h"
#include "stb_image/stb_image.h"

Textures::Textures(const std::string& path) : m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
	stbi_set_flip_vertically_on_load(1); //Flips the texture vertically upside down. OpenGL expects our texture pixels to start from the bottom left of 0,0. Typically, when we load a PNG image, it stores it in a top to bottom format. Thus, we have to flip it on load for OpenGL. If you see your image is upside down, play with this!
	m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
	
	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);

	//We need to specify these 4 things, or we might get a black screen.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //i for Integer. Minification Filter is used if your texture need to be resampled down if it needs to be rendered smaller than it is per pixel. Linear means everything will be linearly resampled.  
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //If we render our texture on an area that is large in pixels than our actual texture size so we have to scaale it up. 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //Horizontal wrap and clamp. We don't want to expand the area.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); //Specify we don't want tiling.

	//0 because it is not a multi level texture, Internal Format is how OpenGL will store your texture data, while format is the format of the data we're providing OpenGL with. 
	//Each of the RGBA channels is an unsigned byte.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
	glBindTexture(GL_TEXTURE_2D, 0); //Unbind once done! :)

	if (m_LocalBuffer)
	{
		stbi_image_free(m_LocalBuffer);
	}
	stbi_set_flip_vertically_on_load(0);
}

Textures::~Textures()
{
	glDeleteTextures(1, &m_RendererID);
}

void Textures::Bind(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot); //I'm going to make the active texture Slot 0. This means the next texture I bind into will be slot 16 until I select another slot again.
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void Textures::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
