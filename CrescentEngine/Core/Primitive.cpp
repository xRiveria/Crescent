#include "CrescentPCH.h"
#include "Primitive.h"
#include <imgui/imgui.h>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

static int temporaryUUID = 0;

float planeVertices[] = {
    // positions            // normals         // texcoords
     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
     25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
};

float cubeVertices[] = {
    //Positions           //Normals            //Texture Coordinates
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};

namespace CrescentEngine
{
    Primitive::Primitive(const PrimitiveShape& primitiveShape)
    {
        SetupPrimitiveBuffers(primitiveShape);
    }

    void Primitive::SetupPrimitiveBuffers(const PrimitiveShape& primitiveShape)
    {
        m_PrimitiveShape = primitiveShape;
        m_PrimitiveObjectID = temporaryUUID++;

        glGenVertexArrays(1, &m_VertexArrayID);
        glGenBuffers(1, &m_VertexBufferID);
        glBindVertexArray(m_VertexArrayID);

        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);

        switch (primitiveShape)
        {
             case PrimitiveShape::PlanePrimitive:
             {
                 glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
                 break;
             }

             case PrimitiveShape::CubePrimitive:
             {
                 glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
                 break;
             }

             case PrimitiveShape::QuadPrimitive:
             {
                 break;
             }
        }

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void Primitive::BindPrimitiveVertexArray() const
    {
        glBindVertexArray(m_VertexArrayID);
    }

    void Primitive::DrawPrimitive(LearnShader& shader)
    {
        shader.UseShader();
        glBindVertexArray(m_VertexArrayID);

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, m_PrimitivePosition);
        shader.SetUniformMat4("model", modelMatrix);

        switch (m_PrimitiveShape)
        {
             case PrimitiveShape::CubePrimitive:
             {
                 glDrawArrays(GL_TRIANGLES, 0, 36);
                 break;
             }

             case PrimitiveShape::PlanePrimitive:
             {
                 glDrawArrays(GL_TRIANGLES, 0, 6);
                 break;
             }

             case PrimitiveShape::QuadPrimitive:
             {
                 break;
             }
        }
        glBindVertexArray(0);
    }

    void Primitive::DrawEditorSettings()
    {
        ImGui::Begin((ConvertPrimitiveEnumToString() + ConvertUUIDToChar()).c_str());
        ImGui::DragFloat3((std::string("Position##") + ConvertUUIDToChar()).c_str(), glm::value_ptr(m_PrimitivePosition));
        ImGui::End();
    }

    std::string Primitive::ConvertUUIDToChar() const
    {
        std::string result;
        std::stringstream convert;
        convert << m_PrimitiveObjectID;
        result = convert.str();

        return convert.str();
    }

    std::string Primitive::ConvertPrimitiveEnumToString() const  //For use with ImGui,
    {
        switch (m_PrimitiveShape)
        {
            case PrimitiveShape::PlanePrimitive:
            {
                return "Plane##";
            }

            case PrimitiveShape::CubePrimitive:
            {
                return "Cube##";
            }

            case PrimitiveShape::QuadPrimitive:
            {
                return "Quad##";
            }
        }
    }

    void TransparentQuad::SetupTransparentQuadBuffers()
    {
        float transparentVertices[] = {
            //Positions          //Texture Coordinates
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
        };

        glGenVertexArrays(1, &m_VertexArrayID);
        glGenBuffers(1, &m_VertexBufferID);
        glBindVertexArray(m_VertexArrayID);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(0);
    }

    void TransparentQuad::DrawTransparentQuad(LearnShader& shader, glm::mat4& modelMatrix)
    {
        shader.UseShader();
        glBindVertexArray(m_VertexArrayID);
        shader.SetUniformMat4("model", modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void TransparentQuad::DrawTransparentQuad(LearnShader& shader, glm::mat4& modelMatrix, Texture2D& texture)
    {
        shader.UseShader();
        glBindVertexArray(m_VertexArrayID);
        texture.BindTexture();

        shader.SetUniformMat4("model", modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
    }
}
