#pragma once
#include <GL/glew.h>
#include "../LearnShader.h"

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
}