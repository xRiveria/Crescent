#include "CrescentPCH.h"
#include "SceneNode.h"

namespace CrescentEngine
{
	SceneNode::SceneNode(unsigned int ID) : m_NodeID(ID)
	{

	}

	SceneNode::~SceneNode()
	{
		//Traverse the list of children and delete accordingly. 
		for (unsigned int i = 0; i < m_ChildNodes.size(); ++i)
		{
			//It should not be possible that a scene node is childed by more than one parent. Thus, we don't need to care about deleting dangling pointers.
			delete m_ChildNodes[i];
		}
	}

	void SceneNode::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		m_Dirty = true;
	}

	void SceneNode::SetRotation(const glm::vec4& rotation)
	{
		m_Rotation = rotation;
		m_Dirty = true;
	}

	void SceneNode::SetScale(const glm::vec3& scale)
	{
		m_Scale = scale;
		m_Dirty = true;
	}

	void SceneNode::SetScale(const float& scale)
	{
		m_Scale = glm::vec3(scale);
		m_Dirty = true;
	}

	glm::vec3 SceneNode::GetWorldPosition()
	{
		glm::mat4 transform = GetTransform();
		glm::vec4 position = transform * glm::vec4(m_Position, 1.0f);
		
		return glm::vec3(position.x, position.y, position.z);
	}

	glm::vec3 SceneNode::GetWorldScale()
	{
		glm::mat4 transform = GetTransform();
		glm::vec3 scale = glm::vec3(transform[0][0], transform[1][1], transform[3][3]);

		//If any of our scale is in the negatives, we make them positive.
		if (scale.x < 0.0f) scale.x *= -1.0f;
		if (scale.y < 0.0f) scale.y *= -1.0f;
		if (scale.z < 0.0f) scale.z *= -1.0f;

		return scale;
	}

	void SceneNode::AddChildNode(SceneNode* node)
	{
		//Check if this child already has a parent. If so, remov ethe node from its current parent. Scene nodes cannot have multiple parents.
		if (node->m_ParentNode)
		{
			node->m_ParentNode->RemoveChildNode(node->m_NodeID);
		}

		node->m_ParentNode = this;
		m_ChildNodes.push_back(node);
	}

	void SceneNode::RemoveChildNode(unsigned int ID)
	{
		auto iterator = std::find(m_ChildNodes.begin(), m_ChildNodes.end(), GetChild(ID));
		if (iterator != m_ChildNodes.end())
		{
			m_ChildNodes.erase(iterator);
		}
	}

	SceneNode* SceneNode::GetChild(unsigned int ID)
	{
		for (unsigned int i = 0; i < m_ChildNodes.size(); ++i)
		{
			if (m_ChildNodes[i]->GetNodeID() == ID)
			{
				return m_ChildNodes[i];
			}
		}

		return nullptr;
	}

	SceneNode* SceneNode::GetChildByIndex(unsigned int index)
	{
		if (index < GetChildNodeCount())
		{
			return m_ChildNodes[index];
		}

		CrescentError("Index is out of bounds of child node count.");
	}

	glm::mat4 SceneNode::GetTransform() 
	{
		if (m_Dirty)
		{
			UpdateTransform(false);
		}

		return m_Transform;
	}

	void SceneNode::UpdateTransform(bool updatePreviousTransform)
	{
		//If specified, store current transform as previous transform (for calculating motion vectors).
		if (updatePreviousTransform)
		{
			m_PreviousTransform = m_Transform;
		}

		//We only do this if the node itself or its parent is flagged as dirty.
		if (m_Dirty)
		{
			//First scale, then rotate, then translation.
			m_Transform = glm::scale(m_Transform, m_Scale);
			m_Transform = glm::rotate(m_Transform, m_Rotation.w, glm::vec3(m_Rotation.x, m_Rotation.y, m_Rotation.z));
			m_Transform = glm::translate(m_Transform, m_Position);

			if (m_ParentNode)
			{
				m_Transform = m_ParentNode->m_Transform * m_Transform;
			}
		}

		for (int i = 0; i < m_ChildNodes.size(); ++i)
		{
			if (m_Dirty)
			{
				m_ChildNodes[i]->m_Dirty = true;
			}

			m_ChildNodes[i]->UpdateTransform(updatePreviousTransform);
		}

		m_Dirty = false;
	}
}