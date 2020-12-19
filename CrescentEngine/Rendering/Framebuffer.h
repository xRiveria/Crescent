#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace CrescentEngine
{
	class Framebuffer
	{
	public:
		Framebuffer() {}
		~Framebuffer();

		void InitializeFramebuffer(int windowWidth, int windowHeight);
		void ResetFramebuffer();
		void BindFramebuffer();
		void UnbindFramebuffer();
		void ResizeFramebuffer(int newWindowWidth, int newWindowHeight);

		int RetrieveFramebufferWidth() const { return m_FramebufferWidth; }
		int RetrieveFramebufferHeight() const { return m_FramebufferHeight; }
		unsigned int RetrieveFramebuffer() const { return m_FramebufferID; }
		unsigned int RetrieveColorAttachment() const { return m_ColorAttachmentID; }

	private:
		void DeleteFramebuffer();

	private:
		unsigned int m_FramebufferID = 0;
		//Attachments are memory locations that can act as a buffer for the framebuffer, think of it as an image.
		//When creating attachments, we have two options to take: textures or renderbuffer objects.
		unsigned int m_ColorAttachmentID = 0;
		unsigned int m_DepthAttachmentID = 0;

		int m_FramebufferWidth = 0;
		int m_FramebufferHeight = 0;

	};
}