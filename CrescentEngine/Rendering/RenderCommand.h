#pragma once
#include <glm/glm.hpp>

namespace Crescent
{
	class Model;
	class Material;

	struct RenderCommand
	{
		glm::mat4 m_Transform;
		Model* m_Mesh;
		Material* m_Material;
	};
}