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
		glGenTextures(2, m_ColorAttachmentIDs);

		for (unsigned int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentIDs[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_FramebufferWidth, m_FramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_ColorAttachmentIDs[i], 0);
		}

		glDrawBuffers(2, m_Attachments);

		//Setup Depth Buffer.
		glGenTextures(1, &m_DepthAttachmentID);
		glActiveTexture(GL_TEXTURE3);
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
			glActiveTexture(0);
		}
		else
		{
			CrescentError("Framebuffer creation failed.");
		}

		glGenFramebuffers(2, m_PingPongFramebuffers);
		glGenTextures(2, m_PingPongColorAttachmentIDs);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_PingPongFramebuffers[i]);
			glBindTexture(GL_TEXTURE_2D, m_PingPongColorAttachmentIDs[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_FramebufferWidth, m_FramebufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PingPongColorAttachmentIDs[i], 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::BindFramebuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
		glViewport(0, 0, m_FramebufferWidth, m_FramebufferHeight);
	}

	void Framebuffer::UnbindFramebuffer()
	{
		glDrawBuffers(1, &m_Attachments[0]);
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
		glDeleteTextures(2, m_ColorAttachmentIDs);
		glDeleteTextures(1, &m_DepthAttachmentID);
	}

	void DepthmapFramebuffer::SetupDepthMapFramebuffer()
	{
		glGenFramebuffers(1, &m_DepthmapFramebufferID);

		//Create Depth Map Texture
		glGenTextures(1, &m_DepthTextureID);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_DepthTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_ShadowWidth, m_ShadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, m_BorderColor);

		//Attach depth texture.
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthmapFramebufferID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTextureID, 0);
		glDrawBuffer(GL_NONE); //Tells OpenGL we're not rendering any color data.
		glReadBuffer(GL_NONE); //Tells OpenGL we're not rendering any color data.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void DepthmapFramebuffer::BindDepthFramebuffer()
	{
		glViewport(0, 0, m_ShadowWidth, m_ShadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthmapFramebufferID);
	}

	void DepthmapFramebuffer::UnbindDepthFramebuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}























