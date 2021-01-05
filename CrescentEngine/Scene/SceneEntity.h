#include <glm/glm.hpp>
#include <vector>

/*
	- Symbolizes a scene entity with a respective UI component. A scene entity contains several default parameters such as a name and transforms.
	- Each entity can have any number of child entities, but there can only ever be one parent. 
*/

namespace Crescent
{
	class Model;
	class Material;

	class SceneEntity
	{
	public:
		SceneEntity(const std::string& entityName, const unsigned int& entityID);

		//Transforms
		void UpdateEntityTransform(bool updatePreviousTransform = false);

		void SetEntityPosition(glm::vec3 newPosition);
		void SetEntityScale(glm::vec3 newScale);
		void SetEntityScale(float newScalar);
		void SetEntityRotation(glm::vec3 newRotation);
		void SetEntityName(const std::string& newName);

		glm::mat4& RetrieveEntityTransform();
		glm::vec3& RetrieveEntityPosition();
		glm::vec3& RetrieveEntityScale();
		glm::vec3& RetrieveEntityRotation();

		std::string RetrieveEntityName() const;

		operator uint32_t() const
		{
			return (uint32_t)m_EntityID;
		}

	public:
		Model* m_Mesh = nullptr;
		Material* m_Material = nullptr;

	private:
		//Scene Information
		std::string m_EntityName = "Entity";
		SceneEntity* m_ParentEntity;
		std::vector<SceneEntity*> m_ChildEntities;

		glm::mat4 m_EntityTransform = glm::mat4(1.0f);
		glm::vec3 m_EntityPosition = glm::vec3(0.0f);
		glm::vec3 m_EntityScale = glm::vec3(1.0f);
		glm::vec3 m_EntityRotation = glm::vec3(0.0f);

		//Each entity is uniquely identified by a 32-bit incrementing unsigned integer.
		bool m_IsTransformDirty = true;
		unsigned int m_EntityID;
	};
}