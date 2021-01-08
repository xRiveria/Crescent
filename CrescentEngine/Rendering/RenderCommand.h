#pragma once
#include <glm/glm.hpp>

namespace Crescent
{
	class Mesh;
	class Material;

	struct RenderCommand
	{
		glm::mat4 m_Transform = glm::mat4(1.0f);
		Mesh* m_Mesh;
		Material* m_Material;
	};
}