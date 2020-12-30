#include "Scene.h"

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
		m_RootNode->AddChild(node);

		return node;
	}

	SceneNode* Scene::CreateSceneNode(Mesh* mesh, Material* material)
	{
		SceneNode* node = new SceneNode(Scene::m_CounterID++);

		node->m_Mesh = mesh;
		node->m_Material = material;

		//Keeps a global reference to this scene node so that we can clear the scene's nodes for memory management at the end of the program or when switching scenes.
		m_RootNode->AddChild(node);

		return node;
	}

	SceneNode* Scene::MakeSceneNode(SceneNode* node)
	{
		return nullptr;
	}

	void Scene::DeleteSceneNode(SceneNode* node)
	{

	}
}
