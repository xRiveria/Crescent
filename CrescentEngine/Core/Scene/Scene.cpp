#include "CrescentPCH.h"
#include "Scene.h"
#include <stack>

namespace CrescentEngine
{
	SceneNode* Scene::m_RootNode = new SceneNode(0);
	unsigned int Scene::m_CounterID = 0;

	void Scene::ClearAllSceneNodes()
	{
		Scene::DeleteSceneNode(m_RootNode);
		Scene::m_RootNode = new SceneNode(0);
	}

	SceneNode* Scene::CreateSceneNode()
	{
		SceneNode* node = new SceneNode(Scene::m_CounterID++);
		//Keep a global reference to this scene node so that we can clear the scene's nodes for memory management at the end of the program or when switching scenes.
		m_RootNode->AddChildNode(node);

		return node;
	}

	SceneNode* Scene::CreateSceneNode(Mesh* mesh, Material* material)
	{
		SceneNode* node = new SceneNode(Scene::m_CounterID++);

		node->m_Mesh = mesh;
		node->m_Material = material;

		//Keeps a global reference to this scene node so that we can clear the scene's nodes for memory management at the end of the program or when switching scenes.
		m_RootNode->AddChildNode(node);

		return node;
	}

	SceneNode* Scene::CreateSceneNode(SceneNode* node)
	{
		SceneNode* newNode = new SceneNode(Scene::m_CounterID++);

		newNode->m_Mesh = node->m_Mesh;
		newNode->m_Material = node->m_Material;
		newNode->m_BoundingBoxMinimum = node->m_BoundingBoxMinimum;
		newNode->m_BoundingBoxMaximum = node->m_BoundingBoxMaximum;

		//Traverse through the list of children and add them correspondingly.
		std::stack<SceneNode*> nodeStack; //LIFO (Last-In, First-Out)
		for (unsigned int i = 0; i < node->GetChildNodeCount(); ++i)
		{
			nodeStack.push(node->GetChildByIndex(i));
		}

		while (!nodeStack.empty())
		{
			SceneNode* childNode = nodeStack.top();
			nodeStack.pop();

			//Simiarly create SceneNode for each child and push to scene node memory list. 
			SceneNode* newChild = new SceneNode(Scene::m_CounterID++);
			newChild->m_Mesh = childNode->m_Mesh;
			newChild->m_Material = childNode->m_Material;
			newChild->m_BoundingBoxMinimum = childNode->m_BoundingBoxMinimum;
			newChild->m_BoundingBoxMaximum = childNode->m_BoundingBoxMaximum;
			
			newNode->AddChildNode(newChild);

			for (unsigned int i = 0; i < childNode->GetChildNodeCount(); ++i)
			{
				nodeStack.push(childNode->GetChildByIndex(i));
			}
		}

		m_RootNode->AddChildNode(newNode);
		return newNode;
	}

	void Scene::DeleteSceneNode(SceneNode* node)
	{
		if (node->GetParentNode())
		{
			node->GetParentNode()->RemoveChildNode(node->GetNodeID());
		}

		//All delete logic is contained within each scene node's destructor.
		delete node;
	}
}
