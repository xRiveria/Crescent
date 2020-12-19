#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../LearnShader.h"

namespace CrescentEngine
{
	class Cubemap
	{
	public:
		Cubemap() {}
		void LoadCubemap(std::vector<std::string> fileLocations);
		void SetupCubemapBuffers();
		void BindCubemap();
		void DrawCubemap(glm::mat4& viewMatix, glm::mat4& projectionMatrix);

		unsigned int RetrieveCubemapID() const { return m_CubemapID; }

	private:
		unsigned int m_CubemapID;
		unsigned int m_CubemapVertexArrayID;
		unsigned int m_CubemapVertexBufferID;
		LearnShader m_CubemapShader;
	};
}