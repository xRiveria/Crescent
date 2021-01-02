#include "CrescentPCH.h"
#include "Framebuffer.h"

namespace Crescent
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

	void Framebuffer::ResetMSAAFramebuffer()
	{
		if (m_MSAAFramebufferID)
		{
			glDeleteFramebuffers(1, &m_MSAAFramebufferID);
			glDeleteTextures(1, &m_MSAAColorAttachmentID);
			glDeleteFramebuffers(1, &m_MSAARenderbufferStorage);
		}

		glGenFramebuffers(1, &m_MSAAFramebufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFramebufferID);

		//Create a multisampled color attachment texture.
		glGenTextures(1, &m_MSAAColorAttachmentID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorAttachmentID);

		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_MultisampleCount, GL_RGBA32F, m_FramebufferWidth, m_FramebufferHeight, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorAttachmentID, 0);

		glGenRenderbuffers(1, &m_MSAARenderbufferStorage);
		glBindRenderbuffer(GL_RENDERBUFFER, m_MSAARenderbufferStorage);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_MultisampleCount, GL_DEPTH24_STENCIL8, m_FramebufferWidth, m_FramebufferHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_MSAARenderbufferStorage);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::ResetFramebuffer(bool multisamplingEnabled)
	{
		ResetMSAAFramebuffer();

		if (m_FramebufferID) //If a framebuffer exists.
		{
			DeleteFramebuffer();
		}

		glGenFramebuffers(1, &m_FramebufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

		//Setup Color Buffer.
		glGenTextures(1, m_ColorAttachmentIDs);

		glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentIDs[0]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_FramebufferWidth, m_FramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachmentIDs[0], 0);

		//glDrawBuffers(2, m_Attachments);

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
			glActiveTexture(GL_TEXTURE0);
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
		//glDrawBuffers(1, &m_Attachments[0]);
		//Once bound, all operations will be done to bound framebuffer. To resume all operations on the main window, we set the default framebuffer active again at Index 0.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::ResizeFramebuffer(int newWindowWidth, int newWindowHeight, bool multisamplingEnabled, int multisamplingCount)
	{
		if (newWindowWidth == 0 || newWindowHeight == 0)
		{
			CrescentInfo("Attempted to resize framebuffer to 0x0.");
			return;
		}

		m_FramebufferWidth = newWindowWidth;
		m_FramebufferHeight = newWindowHeight;

		m_MultisampleCount = multisamplingCount;
		ResetFramebuffer(multisamplingEnabled);	
	}

	void Framebuffer::DeleteFramebuffer()
	{
		glDeleteFramebuffers(1, &m_FramebufferID);
		glDeleteTextures(2, m_ColorAttachmentIDs);
		glDeleteTextures(1, &m_DepthAttachmentID);
	}

	void DepthmapFramebuffer::SetupDepthMapFramebuffer()
	{
		if (m_DepthmapFramebufferID)
		{
			glDeleteFramebuffers(1, &m_DepthmapFramebufferID);
			glDeleteTextures(1, &m_DepthTextureID);
		}
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

	//=========================================================================

	void GBuffer::SetupGBuffer(const int& viewportWidth, const int& viewportHeight)
	{
		m_FramebufferWidth = viewportWidth;
		m_FramebufferHeight = viewportHeight;
		ResetGBuffer();
	}

	unsigned int GBuffer::ResetGBuffer()
	{
		if (m_GBufferID)
		{
			ClearGBuffer();
		}

		glGenFramebuffers(1, &m_GBufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferID);

		//Position Color Buffer. Note that we allocate more memory for Positions and Normals as they ought to be high precision for accurate results. 
		glGenTextures(1, &m_GBufferPositionTextureID);
		glBindTexture(GL_TEXTURE_2D, m_GBufferPositionTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_FramebufferWidth, m_FramebufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBufferPositionTextureID, 0);

		//Normals Color Buffer.
		glGenTextures(1, &m_GBufferNormalsTextureID);
		glBindTexture(GL_TEXTURE_2D, m_GBufferNormalsTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_FramebufferWidth, m_FramebufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBufferNormalsTextureID, 0);

		//Color + Specular Color Buffer.
		glGenTextures(1, &m_GBufferAlbedoSpecularTextureID);
		glBindTexture(GL_TEXTURE_2D, m_GBufferAlbedoSpecularTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_FramebufferWidth, m_FramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBufferAlbedoSpecularTextureID, 0);

		//Tells OpenGL to draw to all our highlighted color attachments.
		glDrawBuffers(3, m_UtilizedGBufferColorAttachments);

		//We also add our render buffer object as depth buffer and check for completeness.
		glGenRenderbuffers(1, &m_GBufferRenderBufferObjectID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_GBufferRenderBufferObjectID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_FramebufferWidth, m_FramebufferHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_GBufferRenderBufferObjectID);

		//Finally, check if our framebuffer is complete.
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			CrescentInfo("GBuffer creation failed.");
		}
		else
		{
			CrescentInfo("GBuffer creation succeeded.");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return m_GBufferID;
	}

	void GBuffer::ResizeFramebuffer(const int& newWidth, const int& newHeight)
	{
		m_FramebufferWidth = newWidth;
		m_FramebufferHeight = newHeight;

		ResetGBuffer();
	}

	void GBuffer::BindGBuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferID);
	}

	void GBuffer::ClearGBuffer()
	{
		glDeleteFramebuffers(1, &m_GBufferID);
		glDeleteTextures(1, &m_GBufferAlbedoSpecularTextureID);
		glDeleteTextures(1, &m_GBufferNormalsTextureID);
		glDeleteTextures(1, &m_GBufferPositionTextureID);
		glDeleteRenderbuffers(1, &m_GBufferRenderBufferObjectID);
	}
}























