#include "CrescentPCH.h"
#include "DefaultShape.h"

namespace CrescentEngine
{
	void Plane::SetupPlaneBuffers()
	{
        float planeVertices[] = {
            // positions          // texture Coords 
             5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

             5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
             5.0f, -0.5f, -5.0f,  2.0f, 2.0f
        };

        glGenVertexArrays(1, &m_VertexArrayID);
        glGenBuffers(1, &m_VertexBufferID);
        glBindVertexArray(m_VertexArrayID);

        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

    void Plane::DrawPlane(LearnShader& shader)
    {
        shader.UseShader();
        glBindVertexArray(m_VertexArrayID);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        shader.SetUniformMat4("model", modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}
