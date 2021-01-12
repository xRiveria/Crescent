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

	void SceneEntity::UpdateEntityTransform(bool updatePreviousTransform)
	{
		if (updatePreviousTransform)
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(m_EntityRotation));
			m_EntityTransform = glm::translate(glm::mat4(1.0f), m_EntityPosition) * rotation * glm::scale(glm::mat4(1.0f), m_EntityScale);
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

	std::string SceneEntity::RetrieveEntityName() const
	{
		return m_EntityName;
	}

	unsigned int SceneEntity::RetrieveEntityID() const
	{
		return m_EntityID;
	}
}