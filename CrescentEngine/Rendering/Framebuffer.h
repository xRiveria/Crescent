#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Crescent
{
	class Framebuffer
	{
	public:
		Framebuffer() {}
		~Framebuffer();

		void InitializeFramebuffer(int windowWidth, int windowHeight);
		void ResetFramebuffer(bool multisamplingEnabled = true);
		void BindFramebuffer();
		void UnbindFramebuffer();
		void ResizeFramebuffer(int newWindowWidth, int newWindowHeight, bool multisamplingEnabled = true, int multisamplingCount = 16);

		int RetrieveFramebufferWidth() const { return m_FramebufferWidth; }
		int RetrieveFramebufferHeight() const { return m_FramebufferHeight; }
		unsigned int RetrieveFramebuffer() const { return m_FramebufferID; }
		unsigned int RetrieveColorAttachment() const { return m_ColorAttachmentIDs[0]; }
		unsigned int RetrieveBloomColorAttachment() const { return m_ColorAttachmentIDs[1]; }

	public:
		unsigned int m_PingPongFramebuffers[2];
		unsigned int m_PingPongColorAttachmentIDs[2];

		unsigned int m_MSAAFramebufferID = 0;
		unsigned int m_MSAAColorAttachmentID = 0;
		unsigned int m_MSAARenderbufferStorage = 0;

	private:
		void DeleteFramebuffer();
		void ResetMSAAFramebuffer();

	private:
		unsigned int m_FramebufferID = 0;
		//Attachments are memory locations that can act as a buffer for the framebuffer, think of it as an image.
		//When creating attachments, we have two options to take: textures or renderbuffer objects.
		unsigned int m_ColorAttachmentIDs[2];
		unsigned int m_Attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		unsigned int m_DepthAttachmentID = 0;
		int m_MultisampleCount = 16;

		int m_FramebufferWidth = 0;
		int m_FramebufferHeight = 0;
	};

	class DepthmapFramebuffer
	{
	public:
		DepthmapFramebuffer() {}
		void SetupDepthMapFramebuffer();
		void BindDepthFramebuffer();
		void UnbindDepthFramebuffer();
		unsigned int RetrieveDepthmapFramebufferID() const { return m_DepthmapFramebufferID; }
		unsigned int RetrieveDepthmapTextureID() const { return m_DepthTextureID; }

	private:
		unsigned int m_DepthmapFramebufferID = 0;
		unsigned int m_DepthTextureID = 0;
		const unsigned int m_ShadowWidth = 1024, m_ShadowHeight = 1024;
		float m_BorderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; //Color of the shadows outside a light source's frustrum. This will cause the GLSL texture function to always return a depth of 1.0f, producing a shadow value of 0.0f.
	};

	class GBuffer
	{
	public:
		void SetupGBuffer(const int& viewportWidth, const int& viewportHeight);
		unsigned int ResetGBuffer();
		void ResizeFramebuffer(const int& newWidth, const int& newHeight);
		void BindGBuffer();
		void ClearGBuffer();

	public:
		unsigned int m_GBufferID;
		unsigned int m_GBufferPositionTextureID;
		unsigned int m_GBufferNormalsTextureID;
		unsigned int m_GBufferAlbedoSpecularTextureID;
		//unsigned int m_GBufferAlbedoTextureID;
		//unsigned int m_GBufferSpecularTextureID;
		unsigned int m_GBufferRenderBufferObjectID;
		unsigned int m_UtilizedGBufferColorAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

		int m_FramebufferWidth = 0;
		int m_FramebufferHeight = 0;
	};
}