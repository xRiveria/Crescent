#include "CrescentPCH.h"
#include "DefaultPrimitives.h"

namespace Crescent
{
    Cube::Cube()
    {
        m_Positions = std::vector<glm::vec3>
        {
            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f,  0.5f, -0.5f),
            glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f,  0.5f, -0.5f),
            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(-0.5f,  0.5f, -0.5f),

            glm::vec3(-0.5f, -0.5f,  0.5f),
            glm::vec3(0.5f, -0.5f,  0.5f),
            glm::vec3(0.5f,  0.5f,  0.5f),
            glm::vec3(0.5f,  0.5f,  0.5f),
            glm::vec3(-0.5f,  0.5f,  0.5f),
            glm::vec3(-0.5f, -0.5f,  0.5f),

            glm::vec3(-0.5f,  0.5f,  0.5f),
            glm::vec3(-0.5f,  0.5f, -0.5f),
            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(-0.5f, -0.5f,  0.5f),
            glm::vec3(-0.5f,  0.5f,  0.5f),

            glm::vec3(0.5f,  0.5f,  0.5f),
            glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f,  0.5f, -0.5f),
            glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f,  0.5f,  0.5f),
            glm::vec3(0.5f, -0.5f,  0.5f),

            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f, -0.5f,  0.5f),
            glm::vec3(0.5f, -0.5f,  0.5f),
            glm::vec3(-0.5f, -0.5f,  0.5f),
            glm::vec3(-0.5f, -0.5f, -0.5f),

            glm::vec3(-0.5f,  0.5f, -0.5f),
            glm::vec3(0.5f,  0.5f,  0.5f),
            glm::vec3(0.5f,  0.5f, -0.5f),
            glm::vec3(0.5f,  0.5f,  0.5f),
            glm::vec3(-0.5f,  0.5f, -0.5f),
            glm::vec3(-0.5f,  0.5f,  0.5f),
        };
        m_UV = std::vector<glm::vec2>{
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),

            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),

            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),

            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
        };

        m_Normals = std::vector<glm::vec3 > {
            glm::vec3(0.0f,  0.0f, -1.0f),
            glm::vec3(0.0f,  0.0f, -1.0f),
            glm::vec3(0.0f,  0.0f, -1.0f),
            glm::vec3(0.0f,  0.0f, -1.0f),
            glm::vec3(0.0f,  0.0f, -1.0f),
            glm::vec3(0.0f,  0.0f, -1.0f),

            glm::vec3(0.0f,  0.0f,  1.0f),
            glm::vec3(0.0f,  0.0f,  1.0f),
            glm::vec3(0.0f,  0.0f,  1.0f),
            glm::vec3(0.0f,  0.0f,  1.0f),
            glm::vec3(0.0f,  0.0f,  1.0f),
            glm::vec3(0.0f,  0.0f,  1.0f),

            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),

            glm::vec3(1.0f,  0.0f,  0.0f),
            glm::vec3(1.0f,  0.0f,  0.0f),
            glm::vec3(1.0f,  0.0f,  0.0f),
            glm::vec3(1.0f,  0.0f,  0.0f),
            glm::vec3(1.0f,  0.0f,  0.0f),
            glm::vec3(1.0f,  0.0f,  0.0f),

            glm::vec3(0.0f, -1.0f,  0.0f),
            glm::vec3(0.0f, -1.0f,  0.0f),
            glm::vec3(0.0f, -1.0f,  0.0f),
            glm::vec3(0.0f, -1.0f,  0.0f),
            glm::vec3(0.0f, -1.0f,  0.0f),
            glm::vec3(0.0f, -1.0f,  0.0f),

            glm::vec3(0.0f,  1.0f,  0.0f),
            glm::vec3(0.0f,  1.0f,  0.0f),
            glm::vec3(0.0f,  1.0f,  0.0f),
            glm::vec3(0.0f,  1.0f,  0.0f),
            glm::vec3(0.0f,  1.0f,  0.0f),
            glm::vec3(0.0f,  1.0f,  0.0f),
        };

        m_Topology = Triangles;
        FinalizeMesh();
    }

    Circle::Circle(unsigned int edgeSegments, unsigned int ringSegments)
    {
        for (unsigned int y = 0; y <= ringSegments; ++y)
        {
            for (unsigned int x = 0; x <= edgeSegments; ++x)
            {
                float xSegment = (float)x / (float)edgeSegments;
                float ringDepth = (float)y / (float)ringSegments;
                const float TAU = 6.28318530717f;
                float xPos = std::cos(xSegment * TAU); // TAU is 2PI
                float yPos = std::sin(xSegment * TAU);

                m_Positions.push_back(glm::vec3(xPos * ringDepth, yPos * ringDepth, 0.0f));
            }
        }

        // indices are exactly the same as for the plane, only the positions differ for a circle
        bool oddRow = false;
        for (int y = 0; y < ringSegments; ++y)
        {
            if (!oddRow) // NOTE(Joey): even rows: y == 0, y == 2; and so on
            {
                for (int x = 0; x <= edgeSegments; ++x)
                {
                    m_Indices.push_back(y * (edgeSegments + 1) + x);
                    m_Indices.push_back((y + 1) * (edgeSegments + 1) + x);
                }
            }
            else
            {
                for (int x = edgeSegments; x >= 0; --x)
                {
                    m_Indices.push_back((y + 1) * (edgeSegments + 1) + x);
                    m_Indices.push_back(y * (edgeSegments + 1) + x);
                }
            }
            oddRow = !oddRow;
        }

        m_Topology = TriangleStrips;
        FinalizeMesh();
    }
}
