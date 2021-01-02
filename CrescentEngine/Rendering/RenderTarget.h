#pragma once
#include "Texture.h"

namespace Crescent
{
	//Render targets are framebuffers.

	class RenderTarget
	{
	public:
		RenderTarget(unsigned int renderTargetWidth, unsigned int renderTargetHeight, GLenum renderTargetDataType = GL_UNSIGNED_BYTE, unsigned int colorAttachmentsCount = 1, bool hasDepthAndStencilAttachments = true);

		Texture2D* RetrieveDepthAndStencilAttachment();
		Texture2D* RetrieveColorAttachment(unsigned int index);

		void ResizeRenderTarget(unsigned int newWidth, unsigned int newHeight);
		void SetRenderTarget(GLenum target);

	public:
		unsigned int m_RenderTargetID;
		unsigned int m_RenderTargetWidth;
		unsigned int m_RenderTargetHeight;
		GLenum m_RenderTargetDataType;
		
		bool m_RenderTargetHasDepthAndStencil;

	private:
		GLenum m_RenderTargetType = GL_TEXTURE_2D;
		Texture2D m_DepthAndStencilAttachment;
		std::vector<Texture2D> m_ColorAttachments;
	};
}