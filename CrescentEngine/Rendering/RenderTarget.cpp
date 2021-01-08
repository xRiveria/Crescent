#include "CrescentPCH.h"
#include "RenderTarget.h"

namespace Crescent
{
	RenderTarget::RenderTarget(unsigned int framebufferWidth, unsigned int framebufferHeight, GLenum framebufferDataType, unsigned int colorAttachmentCount, bool hasDepthAndStencilAttachment)
	{
		m_FramebufferWidth = framebufferWidth;
		m_FramebufferHeight = framebufferHeight;
		m_FramebufferDataType = framebufferDataType;

		glGenFramebuffers(1, &m_FramebufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
		//Generate all requested color attachments.
		for (unsigned int i = 0; i < colorAttachmentCount; i++)
		{
			Texture texture;
			texture.m_TextureMinificationFilter = GL_LINEAR;
			texture.m_TextureMagnificationFilter = GL_LINEAR;
			texture.m_TextureWrapS = GL_CLAMP_TO_EDGE;
			texture.m_TextureWrapT = GL_CLAMP_TO_EDGE;
			texture.m_MipmappingEnabled = false;

			GLenum internalFormat = GL_RGBA;
			if (framebufferDataType == GL_HALF_FLOAT)
			{
				internalFormat = GL_RGBA16F;
			}
			else if (framebufferDataType == GL_FLOAT)
			{
				internalFormat = GL_RGBA32F;
			}
			texture.GenerateTexture(framebufferWidth, framebufferHeight, internalFormat, GL_RGBA, framebufferDataType, 0);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture.RetrieveTextureID(), 0);
			m_ColorAttachments.push_back(texture);
		}

		//Then, proceed to generate Depth/Stencil texture if requested.
		m_HasDepthAndStencilAttachments = hasDepthAndStencilAttachment;

		if (hasDepthAndStencilAttachment)
		{
			Texture texture;
			texture.m_TextureMinificationFilter = GL_LINEAR;
			texture.m_TextureMagnificationFilter = GL_LINEAR;
			texture.m_TextureWrapS = GL_CLAMP_TO_EDGE;
			texture.m_TextureWrapT = GL_CLAMP_TO_EDGE;
			texture.m_MipmappingEnabled = false;
			texture.GenerateTexture(framebufferWidth, framebufferHeight, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture.RetrieveTextureID(), 0);
			m_DepthAndStencilAttachment = texture;
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			CrescentInfo("Framebuffer creation failed - Not complete!");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Texture* RenderTarget::RetrieveDepthAndStencilAttachment()
	{
		return &m_DepthAndStencilAttachment;
	}

	Texture* RenderTarget::RetrieveColorAttachment(unsigned int attachmentIndex)
	{
		if (attachmentIndex < m_ColorAttachments.size())
		{
			return &m_ColorAttachments[attachmentIndex];
		}
		else
		{
			CrescentError("Requested a Color Attachment that was out of bounds.");
			return nullptr;
		}
	}

	void RenderTarget::ResizeRenderTarget(unsigned int newWidth, unsigned int newHeight)
	{
		m_FramebufferWidth = newWidth;
		m_FramebufferHeight = newHeight;

		for (unsigned int i = 0; i < m_ColorAttachments.size(); i++)
		{
			m_ColorAttachments[i].ResizeTexture(newWidth, newHeight);
		}

		//Generate depth/stencil textures if needed.
		if (m_HasDepthAndStencilAttachments)
		{
			m_DepthAndStencilAttachment.ResizeTexture(newWidth, newHeight);
		}
	}

	void RenderTarget::SetRenderTarget(GLenum target)
	{
		m_FramebufferTarget = target;
	}
}