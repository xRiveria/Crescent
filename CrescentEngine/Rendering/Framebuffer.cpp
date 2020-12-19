#include "CrescentPCH.h"
#include "Framebuffer.h"

namespace CrescentEngine
{
	Framebuffer::~Framebuffer()
	{
		DeleteFramebuffer();
	}

	void Framebuffer::InitializeFramebuffer(int windowWidth, int windowHeight)
	{
		m_FramebufferWidth = windowWidth;
		m_FramebufferHeight = windowHeight;

		ResetFramebuffer();
	}

	void Framebuffer::ResetFramebuffer()
	{
		if (m_FramebufferID) //If a framebuffer exists.
		{
			DeleteFramebuffer();
		}

		glGenFramebuffers(1, &m_FramebufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

		//Setup Color Buffer.
		glGenTextures(1, &m_ColorAttachmentID);
		glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_FramebufferWidth, m_FramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachmentID, 0);

		//Setup Depth Buffer.
		glGenTextures(1, &m_DepthAttachmentID);
		glBindTexture(GL_TEXTURE_2D, m_DepthAttachmentID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_FramebufferWidth, m_FramebufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_FramebufferWidth, m_FramebufferHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachmentID, 0);
		//glFramebufferTexture2D(GL_FRAGMENT_TEXTURE, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachmentID, 0);

		//Once all requirements are completed, we can check if the Framebyffer is successfully completed by doing the following.
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		{
			CrescentInfo("All requirements passed. Successfully created Framebuffer.")
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else
		{
			CrescentError("Framebuffer creation failed.");
		}
	}

	void Framebuffer::BindFramebuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
		glViewport(0, 0, m_FramebufferWidth, m_FramebufferHeight);
	}

	void Framebuffer::UnbindFramebuffer()
	{
		//Once bound, all operations will be done to bound framebuffer. To resume all operations on the main window, we set the default framebuffer active again at Index 0.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::ResizeFramebuffer(int newWindowWidth, int newWindowHeight)
	{
		if (newWindowWidth == 0 || newWindowHeight == 0)
		{
			CrescentInfo("Attempted to resize framebuffer to 0x0.");
			return;
		}

		m_FramebufferWidth = newWindowWidth;
		m_FramebufferHeight = newWindowHeight;

		ResetFramebuffer();
	}

	void Framebuffer::DeleteFramebuffer()
	{
		glDeleteFramebuffers(1, &m_FramebufferID);
		glDeleteTextures(1, &m_ColorAttachmentID);
		glDeleteTextures(1, &m_DepthAttachmentID);
	}
}























