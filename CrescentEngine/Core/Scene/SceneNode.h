#pragma once
#include "../Rendering/Material.h"
#include "../Models/Mesh.h"
#include <glm/glm.hpp>
//Not an actual ECS implementation. 

namespace CrescentEngine
{
	class SceneNode
	{
	public:
		//Each node contains relevant render state.
		Mesh* m_Mesh;
		Material* m_Material;

		// bounding box 
		glm::vec3 m_BoxMinimum = glm::vec3(-99999.0f);
		glm::vec3 m_BoxMaximum = glm::vec3(99999.0f);

	public:
		SceneNode(unsigned int ID);
		void AddChild(SceneNode* node);
	};
}