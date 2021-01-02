#pragma once
#include "glm/glm.hpp"
#include "../Models/Mesh.h"
#include "Material.h"

namespace Crescent
{
	//All render state required for submitting a render command.
	struct RenderCommand
	{
		glm::mat4 m_Transform;
		glm::mat4 m_PreviousTransform;
		Mesh* m_Mesh;
		Material* m_Material;
		glm::vec3 m_BoundingBoxMinimum;
		glm::vec3 m_BoundingBoxMaximum;
	};
}