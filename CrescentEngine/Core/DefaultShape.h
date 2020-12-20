#pragma once
#include <GL/glew.h>
#include "../LearnShader.h"
#include "../Rendering/Texture.h"

//To combine into one huge Primitive class when the time comes.

namespace CrescentEngine
{
	class Plane
	{
	public:
		Plane() {}
		void SetupPlaneBuffers();
		void BindPlaneVertexArray() const { glBindVertexArray(m_VertexArrayID); }
		void DrawPlane(LearnShader& shader);

	private:
		unsigned int m_VertexArrayID = 0;
		unsigned int m_VertexBufferID = 0;
	};

	class TransparentQuad
	{
	public:
		TransparentQuad() {}
		void SetupTransparentQuadBuffers();
		void BindTransparentQuadVertexArray() const { glBindVertexArray(m_VertexArrayID); }

		void DrawTransparentQuad(LearnShader& shader, glm::mat4& modelMatrix);
		void DrawTransparentQuad(LearnShader& shader, glm::mat4& modelMatrix, Texture2D& texture);	

	private:
		unsigned int m_VertexArrayID = 0;
		unsigned int m_VertexBufferID = 0;
	};
}