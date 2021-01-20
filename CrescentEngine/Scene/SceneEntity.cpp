#include "CrescentPCH.h"
#include "SceneEntity.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Crescent
{
	SceneEntity::SceneEntity(const std::string& entityName, const unsigned int& entityID) : m_EntityName(entityName), m_EntityID(entityID)
	{

	}

	void SceneEntity::AddChildEntity(SceneEntity* childEntity)
	{
		//Check if this child already has a parent. If so, first remove this scene node from its current parent. Scene nodes cannot exist under multiple parents.
		if (childEntity->m_ParentEntity != nullptr)
		{
			childEntity->m_ParentEntity->RemoveChildEntity(childEntity->m_EntityID);
		}

		childEntity->m_ParentEntity = this;
		m_ChildEntities.push_back(childEntity);
	}

	void SceneEntity::RemoveChildEntity(unsigned int entityID)
	{
		auto iterator = std::find(m_ChildEntities.begin(), m_ChildEntities.end(), RetrieveChildEntity(entityID));
		if (iterator != m_ChildEntities.end())
		{
			m_ChildEntities.erase(iterator);
		}
	}

	void SceneEntity::UpdateEntityTransform(bool updatePreviousTransform)
	{
		if (m_IsTransformDirty)
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(m_EntityRotation));
			m_EntityTransform = glm::translate(glm::mat4(1.0f), m_EntityPosition) * rotation * glm::scale(glm::mat4(1.0f), m_EntityScale);

			if (m_ParentEntity != nullptr)
			{
				m_EntityTransform = m_ParentEntity->m_EntityTransform * m_EntityTransform;
			}
		}

		for (int i = 0; i < m_ChildEntities.size(); i++)
		{
			if (m_IsTransformDirty)
			{
				m_ChildEntities[i]->m_IsTransformDirty = true;
			}
			m_ChildEntities[i]->UpdateEntityTransform(updatePreviousTransform);
		}

		m_IsTransformDirty = false;
	}

	void SceneEntity::SetEntityPosition(glm::vec3 newPosition)
	{
		m_EntityPosition = newPosition;
		m_IsTransformDirty = true;
	}

	void SceneEntity::SetEntityScale(glm::vec3 newScale)
	{
		m_EntityScale = newScale;
		m_IsTransformDirty = true;
	}

	void SceneEntity::SetEntityScale(float newScalar)
	{
		m_EntityScale = glm::vec3(newScalar, newScalar, newScalar);
		m_IsTransformDirty = true;
	}

	void SceneEntity::SetEntityRotation(glm::vec3 newRotation)
	{
		m_EntityRotation = newRotation;
		m_IsTransformDirty = true;
	}

	void SceneEntity::SetEntityName(const std::string& newName)
	{
		m_EntityName = newName;
	}

	glm::mat4& SceneEntity::RetrieveEntityTransform()
	{
		if (m_IsTransformDirty)
		{
			UpdateEntityTransform(true);
		}

		return m_EntityTransform;
	}

	glm::vec3& SceneEntity::RetrieveEntityPosition()
	{
		return m_EntityPosition;
	}

	glm::vec3& SceneEntity::RetrieveEntityScale()
	{
		return m_EntityScale;
	}

	glm::vec3& SceneEntity::RetrieveEntityRotation()
	{
		return m_EntityRotation;
	}

	SceneEntity* SceneEntity::RetrieveChildEntity(unsigned int entityID)
	{
		for (unsigned int i = 0; i < m_ChildEntities.size(); i++)
		{
			if (m_ChildEntities[i]->m_EntityID == entityID)
			{
				return m_ChildEntities[i];
			}
		}
		return nullptr;
	}

	SceneEntity* SceneEntity::RetrieveChildByIndex(unsigned int entityIndex)
	{
		return m_ChildEntities[entityIndex];
	}

	std::string SceneEntity::RetrieveEntityName() const
	{
		return m_EntityName;
	}

	unsigned int SceneEntity::RetrieveEntityID() const
	{
		return m_EntityID;
	}
}