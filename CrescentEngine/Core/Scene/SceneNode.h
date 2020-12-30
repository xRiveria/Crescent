#pragma once
#include "../Rendering/Material.h"
#include "../Models/Mesh.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace CrescentEngine
{
	/*
		Not an actual ECS implementation. 
		An individual scene node that links a mesh to a material to render the mesh with while at the same time maintaining a parent-child hierarchy for transform relations.
		Each node can have any number of children (via a linked list) and represents a single renderable entity in a large scene.
	*/

	class SceneNode
	{
	public:
		SceneNode(unsigned int ID);
		~SceneNode();

		//Scene Node Transforms
		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::vec4& rotation);
		void SetScale(const glm::vec3& scale);
		void SetScale(const float& scale);

		glm::vec3 GetLocalPosition() const { return m_Position; }
		glm::vec4 GetLocalRotation() const { return m_Rotation; }
		glm::vec3 GetLocalScale() const { return m_Scale; }
		glm::vec3 GetWorldPosition();
		glm::vec3 GetWorldScale();

		//Scene Graph
		unsigned int GetNodeID() const { return m_NodeID; }
		void AddChildNode(SceneNode* node);
		void RemoveChildNode(unsigned int ID);  //Think of proper way to uniquely identify child nodes, perhaps with incrementing node ID or string hash ID.
		std::vector<SceneNode*> GetChildNodes() const { return m_ChildNodes; }
		unsigned int GetChildNodeCount() const { return m_ChildNodes.size(); }
		SceneNode* GetChild(unsigned int ID);
		SceneNode* GetChildByIndex(unsigned int index);
		SceneNode* GetParentNode() const { return m_ParentNode; }

		//Returns the transform of the current node combined with its parent(s)'s transform.
		glm::mat4 GetTransform();
		glm::mat4 GetPreviousTransform() const { m_PreviousTransform; }

		//Recalculates this node and its children's transform components if its parent or the node itself is dirty.
		void UpdateTransform(bool updatePreviousTransform = false);

	public:
		//Each node contains relevant render state.
		Mesh* m_Mesh;
		Material* m_Material;

		//Bounding Box
		glm::vec3 m_BoxMinimum = glm::vec3(-99999.0f);
		glm::vec3 m_BoxMaximum = glm::vec3(99999.0f);

	private:
		std::vector<SceneNode*> m_ChildNodes;
		SceneNode* m_ParentNode;

		//Per-Node Transform (With Parent-Child Relationship)
		glm::mat4 m_Transform;
		glm::mat4 m_PreviousTransform;
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec4 m_Rotation; //Axis-Angles for now. Quaternions coming soon.
		glm::vec3 m_Scale = glm::vec3(1.0f);

		//Mark the current node's transform as dirty if it needs to be recalculated this frame. 
		bool m_Dirty;

		//Each node is uniquely identified by a 32-bit incrementing unsigned integer.
		unsigned int m_NodeID;

		static unsigned int m_CounterID;
	};
}