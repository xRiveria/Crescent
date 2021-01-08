#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "../Shading/Texture.h"

namespace Crescent
{
	class Renderer;

	class RenderTarget
	{
		friend Renderer;

	public:
		RenderTarget(unsigned int framebufferWidth, unsigned int framebufferHeight, GLenum framebufferDataType = GL_UNSIGNED_BYTE, unsigned int colorAttachmentCount = 1, bool hasDepthAndStencilAttachment = true);
		Texture* RetrieveDepthAndStencilAttachment();
		Texture* RetrieveColorAttachment(unsigned int attachmentIndex);

		void ResizeRenderTarget(unsigned int newWidth, unsigned int newHeight);
		void SetRenderTarget(GLenum target);

	public:
		unsigned int m_FramebufferID;
		unsigned int m_FramebufferWidth;
		unsigned int m_FramebufferHeight;
		GLenum m_FramebufferDataType;

		bool m_HasDepthAndStencilAttachments;

	private:
		GLenum m_FramebufferTarget = GL_TEXTURE_2D;
		Texture m_DepthAndStencilAttachment;
		std::vector<Texture> m_ColorAttachments;
	};
}