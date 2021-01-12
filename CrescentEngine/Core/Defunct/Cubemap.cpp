#include "CrescentPCH.h"
#include "Cubemap.h"
#include <GL/glew.h>
#include <stb_image/stb_image.h>

namespace Crescent
{
    void Cubemap::LoadCubemap(std::vector<std::string> fileLocations)
    {
        m_CubemapShader.CreateShaders("Resources/Shaders/CubemapVertex.shader", "Resources/Shaders/CubemapFragment.shader");
        glGenTextures(1, &m_CubemapID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < fileLocations.size(); i++)
        {         
            unsigned char* data = stbi_load(fileLocations[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                GLenum format;
                if (nrChannels == 1)
                {
                    format = GL_RED;
                }
                else if (nrChannels == 3)
                {
                    format = GL_RGB;
                }
                else if (nrChannels == 4)
                {
                    format = GL_RGBA;
                }

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                CrescentInfo("Failed to load cubemap.");
                stbi_image_free(data);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        m_CubemapShader.UseShader();
        m_CubemapShader.SetUniformInteger("skybox", 0);
    }

    void Cubemap::SetupCubemapBuffers()
    {
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        glGenVertexArrays(1, &m_CubemapVertexArrayID);
        glGenBuffers(1, &m_CubemapVertexBufferID);
        glBindVertexArray(m_CubemapVertexArrayID);

        glBindBuffer(GL_ARRAY_BUFFER, m_CubemapVertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    void Cubemap::BindCubemap()
    {
        glBindVertexArray(m_CubemapVertexArrayID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID);
    }

    void Cubemap::DrawCubemap(glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
    {
        glDepthFunc(GL_LEQUAL); //Change depth function so that depth test passes when values are equal to depth buffer's content.
        m_CubemapShader.UseShader();
        glm::mat4 translationStrippedViewMatrix = glm::mat4(glm::mat3(viewMatrix)); //Strip translation from the view matrix.
        m_CubemapShader.SetUniformMat4("view", translationStrippedViewMatrix);
        m_CubemapShader.SetUniformMat4("projection", projectionMatrix);

        BindCubemap();

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        //m_CubemapShader.UnbindShader();

        glDepthFunc(GL_LESS); //Set depth function back to default.
    }
}
