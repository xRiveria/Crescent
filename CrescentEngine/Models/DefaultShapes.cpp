#include "CrescentPCH.h"
#include "DefaultShapes.h"

namespace Crescent
{
    const float PI = 3.14159265359f;
    const float TAU = 6.28318530717f;

	Cube::Cube()
	{
        m_Positions = std::vector<glm::vec3>{
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

        m_Normals = std::vector<glm::vec3>{
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

	Plane::Plane(unsigned int xSegments, unsigned int ySegments)
	{
        bool oddRow = false;

        float dX = 1.0f / xSegments;
        float dY = 1.0f / ySegments;

        for (int y = 0; y <= ySegments; ++y)
        {
            for (int x = 0; x <= xSegments; ++x)
            {
                m_Positions.push_back(glm::vec3(dX * x * 2.0f - 1.0f, dY * y * 2.0f - 1.0f, 0.0f));
                m_UV.push_back(glm::vec2(dX * x, 1.0f - y * dY));
                m_Normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
            }
        }

        for (int y = 0; y < ySegments; ++y)
        {
            if (!oddRow) // Even rows: y == 0, y == 2; and so on.
            {
                for (int x = 0; x <= xSegments; ++x)
                {
                    m_Indices.push_back(y * (xSegments + 1) + x);
                    m_Indices.push_back((y + 1) * (xSegments + 1) + x);
                }
            }
            else
            {
                for (int x = xSegments; x >= 0; --x)
                {
                    m_Indices.push_back((y + 1) * (xSegments + 1) + x);
                    m_Indices.push_back(y * (xSegments + 1) + x);
                }
            }
            oddRow = !oddRow;
        }

        m_Topology = TriangleStrip;
        FinalizeMesh();
	}

	Torus::Torus(float r1, float r2, unsigned int numSteps1, unsigned int numSteps2)
	{
        // We generate an additional minor ring segment as we can't directly connect to the first minor ring as the last set of vertices require unique texture coordinates.

        m_Positions.resize((numSteps1 + 1) * (numSteps2 + 1));
        m_Normals.resize((numSteps1 + 1) * (numSteps2 + 1));
        m_UV.resize((numSteps1 + 1) * (numSteps2 + 1));

        //First, calculate all points for the major ring on the XY plane (in textbook mathematics, the z-axis is considered to be the up axis).
        std::vector<glm::vec3> p(numSteps1 + 1);
        float a = 0.0f;
        float step = 2.0f * PI / numSteps1;
        for (int i = 0; i <= numSteps1; ++i)
        {
            float x = cos(a) * r1;
            float y = sin(a) * r1;
            p[i].x = x;
            p[i].y = y;
            p[i].z = 0.0f;
            a += step;
        }

        // Generate all the vertices, UVs, Normals (and Tangents/Bitangents):
        for (int i = 0; i <= numSteps1; ++i)
        {
            //The basis vectors of the ring equal the difference vector between the minorRing center and the donut's center position (which equals the origin (0, 0, 0)) and the positive Z-axis.
            glm::vec3 u = glm::normalize(glm::vec3(0.0f) - p[i]) * r2; // Could be p[i] also        
            glm::vec3 v = glm::vec3(0.0f, 0.0f, 1.0f) * r2;

            //Create the vertices of each minor ring segment:
            float a = 0.0f;
            float step = 2.0f * PI / numSteps2;
            for (int j = 0; j <= numSteps2; ++j)
            {
                float c = cos(a);
                float s = sin(a);

                m_Positions[i * (numSteps2 + 1) + j] = p[i] + c * u + s * v;
                m_UV[i * (numSteps2 + 1) + j].x = ((float)i) / ((float)numSteps1) * TAU; //Multiply by TAU to keep UVs symmetric along both axes.
                m_UV[i * (numSteps2 + 1) + j].y = ((float)j) / ((float)numSteps2);
                m_Normals[i * (numSteps2 + 1) + j] = glm::normalize(c * u + s * v);
                a += step;
            }
        }

        //Generate the indicies for a triangle topology:
        m_Indices.resize(numSteps1 * numSteps2 * 6);

        int index = 0;
        for (int i = 0; i < numSteps1; ++i)
        {
            int i1 = i;
            int i2 = (i1 + 1);

            for (int j = 0; j < numSteps2; ++j)
            {
                int j1 = j;
                int j2 = (j1 + 1);

                m_Indices[index++] = i1 * (numSteps2 + 1) + j1;
                m_Indices[index++] = i1 * (numSteps2 + 1) + j2;
                m_Indices[index++] = i2 * (numSteps2 + 1) + j1;

                m_Indices[index++] = i2 * (numSteps2 + 1) + j2;
                m_Indices[index++] = i2 * (numSteps2 + 1) + j1;
                m_Indices[index++] = i1 * (numSteps2 + 1) + j2;
            }
        }

        m_Topology = Triangles;
        FinalizeMesh();
	}

	Sphere::Sphere(unsigned int xSegments, unsigned int ySegments)
	{
        for (unsigned int y = 0; y <= ySegments; ++y)
        {
            for (unsigned int x = 0; x <= xSegments; ++x)
            {
                float xSegment = (float)x / (float)xSegments;
                float ySegment = (float)y / (float)ySegments;
                float xPos = std::cos(xSegment * TAU) * std::sin(ySegment * PI); // TAU is 2PI.
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * TAU) * std::sin(ySegment * PI);

                m_Positions.push_back(glm::vec3(xPos, yPos, zPos));
                m_UV.push_back(glm::vec2(xSegment, ySegment));
                m_Normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;

        for (int y = 0; y < ySegments; ++y)
        {
            for (int x = 0; x < xSegments; ++x)
            {
                m_Indices.push_back((y + 1) * (xSegments + 1) + x);
                m_Indices.push_back(y * (xSegments + 1) + x);
                m_Indices.push_back(y * (xSegments + 1) + x + 1);

                m_Indices.push_back((y + 1) * (xSegments + 1) + x);
                m_Indices.push_back(y * (xSegments + 1) + x + 1);
                m_Indices.push_back((y + 1) * (xSegments + 1) + x + 1);
            }
        }

        m_Topology = Triangles;
        FinalizeMesh();
	}

	Quad::Quad()
	{
        m_Positions =
        {
            { -1.0f,  1.0f, 0.0f, },
            { -1.0f, -1.0f, 0.0f, },
            {  1.0f,  1.0f, 0.0f, },
            {  1.0f, -1.0f, 0.0f, },
        };

        m_UV = {
            { 0.0f, 1.0f, },
            { 0.0f, 0.0f, },
            { 1.0f, 1.0f, },
            { 1.0f, 0.0f, },
        };

        m_Topology = TriangleStrip;
        FinalizeMesh();
	}

	Quad::Quad(float quadWidth, float quadHeight)
	{
        m_Positions =
        {
            { -quadWidth,  quadHeight, 0.0f, },
            { -quadWidth, -quadHeight, 0.0f, },
            {  quadWidth,  quadHeight, 0.0f, },
            {  quadWidth, -quadHeight, 0.0f, },
        };

        m_UV = {
            { 0.0f, 1.0f, },
            { 0.0f, 0.0f, },
            { 1.0f, 1.0f, },
            { 1.0f, 0.0f, },
        };

        m_Topology = TriangleStrip;
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
                float xPos = std::cos(xSegment * TAU); // TAU is 2PI.
                float yPos = std::sin(xSegment * TAU);

               m_Positions.push_back(glm::vec3(xPos * ringDepth, yPos * ringDepth, 0.0f));
            }
        }

        //Indices are exactly the same as for the plane, only the positions differ for a circle.
        bool oddRow = false;
        for (int y = 0; y < ringSegments; ++y)
        {
            if (!oddRow) //Even rows: y == 0, y == 2; and so on.
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

        m_Topology = TriangleStrip;
        FinalizeMesh();
	}

	LineStrip::LineStrip(float width, unsigned int segments)
	{
        float deltaX = 1.0f / segments;
        for (int i = 0; i <= segments; ++i)
        {
            //X: range from -0.5f - 0.5f | Y: range from -0.5f - 0.5f.
            m_Positions.push_back({ -0.5f + (float)i * deltaX,  0.5f * width, 0.0f });
            m_Positions.push_back({ -0.5f + (float)i * deltaX, -0.5f * width, 0.0f });

            m_UV.push_back({ (float)i * deltaX, 1.0f });
            m_UV.push_back({ (float)i * deltaX, 0.0f });
        }

        m_Topology = TriangleStrip;

        FinalizeMesh();
	}
}