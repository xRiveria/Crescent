#include "CrescentPCH.h"
#include "RenderTarget.h"

namespace Crescent
{
	RenderTarget::RenderTarget(unsigned int renderTargetWidth, unsigned int renderTargetHeight, GLenum renderTargetDataType, unsigned int colorAttachmentsCount, bool hasDepthAndStencilAttachments)
	{
		m_RenderTargetWidth = renderTargetWidth;
		m_RenderTargetHeight = renderTargetHeight;
		m_RenderTargetDataType = renderTargetDataType;

		glGenFramebuffers(1, &m_RenderTargetID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RenderTargetID);

		//Generate all requested color attachments.
		for (unsigned int i = 0; i < colorAttachmentsCount; ++i)
		{
			Texture2D texture;
			texture.m_MinificationFilter = GL_LINEAR;
			texture.m_MagnificationFilter = GL_LINEAR;
			texture.m_WrapS = GL_CLAMP_TO_EDGE;
			texture.m_WrapT = GL_CLAMP_TO_EDGE;
			texture.m_MipmappingEnabled = false;

			GLenum internalFormat = GL_RGBA;
			if (renderTargetDataType == GL_HALF_FLOAT)
			{
				internalFormat = GL_RGBA16F;
			}
			else if (renderTargetDataType == GL_FLOAT)
			{
				internalFormat = GL_RGBA32F;
			}

			texture.GenerateTexture(renderTargetWidth, renderTargetHeight, internalFormat, GL_RGBA, renderTargetDataType, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture.m_TextureID, 0);
			m_ColorAttachments.push_back(texture);
		}

		//Generate Depth/Stencil texture if requested.
		m_RenderTargetHasDepthAndStencil = hasDepthAndStencilAttachments;
		if (hasDepthAndStencilAttachments)
		{
			Texture2D texture;
			texture.m_MinificationFilter = GL_LINEAR;
			texture.m_MagnificationFilter = GL_LINEAR;
			texture.m_WrapS = GL_CLAMP_TO_EDGE;
			texture.m_WrapT = GL_CLAMP_TO_EDGE;
			texture.m_MipmappingEnabled = false;
			texture.GenerateTexture(renderTargetWidth, renderTargetHeight, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture.m_TextureID, 0);
			m_DepthAndStencilAttachment = texture;
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			CrescentError("Framebuffer not complete. Error!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Texture2D* RenderTarget::RetrieveDepthAndStencilAttachment() 
	{
		return &m_DepthAndStencilAttachment;
	}

	Texture2D* RenderTarget::RetrieveColorAttachment(unsigned int index) 
	{
		if (index < m_ColorAttachments.size())
		{
			return &m_ColorAttachments[index];
		}
		else
		{
			CrescentError("Render Target color attachment requested but not avaliable: " + std::to_string(index));
			return nullptr;
		}
	}

	void RenderTarget::ResizeRenderTarget(unsigned int newWidth, unsigned int newHeight)
	{
		m_RenderTargetWidth = newWidth;
		m_RenderTargetHeight = newHeight;

		for (unsigned int i = 0; i < m_ColorAttachments.size(); ++i)
		{
			m_ColorAttachments[i].ResizeTexture(newWidth, newHeight);
		}

		//Generate depth/stencil if requested.
		if (m_RenderTargetHasDepthAndStencil)
		{
			m_DepthAndStencilAttachment.ResizeTexture(newWidth, newHeight);
		}
	}

	void RenderTarget::SetRenderTarget(GLenum target)
	{
		m_RenderTargetType = target;
	}
}