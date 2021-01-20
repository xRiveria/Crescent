#include "CrescentPCH.h"
#include "Scene.h"
#include "SceneEntity.h"
#include "Entities/Skybox.h"
#include <stack>

namespace Crescent
{
	unsigned int Scene::m_SceneEntityCounterID = 0;

	Scene::Scene(bool isEmptyScene)
	{
		ConstructDefaultScene();
	}

	void Scene::ClearScene()
	{
		for (int i = 0; i < m_SceneEntities.size(); i++)
		{
			DeleteSceneEntity(m_SceneEntities[i]);
		}
	}

	SceneEntity* Scene::ConstructNewEntity()
	{
		SceneEntity* newEntity = new SceneEntity("Empty Entity", Scene::m_SceneEntityCounterID++);
		m_SceneEntities.push_back(newEntity);

		return newEntity;
	}

	SceneEntity* Scene::ConstructNewEntity(Mesh* mesh, Material* material)
	{
		SceneEntity* newEntity = new SceneEntity("Model", Scene::m_SceneEntityCounterID++);
		
		newEntity->m_Mesh = mesh;
		newEntity->m_Material = material;

		m_SceneEntities.push_back(newEntity);
		
		return newEntity;
	}

	SceneEntity* Scene::ConstructNewEntity(SceneEntity* sceneEntity)
	{
		SceneEntity* newEntity = new SceneEntity("MeshHehe", Scene::m_SceneEntityCounterID++);

		newEntity->m_Mesh = sceneEntity->m_Mesh;
		newEntity->m_Material = sceneEntity->m_Material;

		//Traverse through the list of children and add them accordingly.
		std::stack<SceneEntity*> nodeStack;
		for (unsigned int i = 0; i < sceneEntity->RetrieveChildCount(); i++)
		{
			nodeStack.push(sceneEntity->RetrieveChildByIndex(i));
		}
		while (!nodeStack.empty())
		{
			SceneEntity* child = nodeStack.top();
			nodeStack.pop();
			//Similarly, create SceneNode for each child and push to scene node memory list.
			SceneEntity* newChild = new SceneEntity("MeshHehe", Scene::m_SceneEntityCounterID++);
			newChild->m_Mesh = child->m_Mesh;
			newChild->m_Material = child->m_Material;
			newEntity->AddChildEntity(newChild);

			for (unsigned int i = 0; i < child->RetrieveChildCount(); i++)
			{
				nodeStack.push(child->RetrieveChildByIndex(i));
			}
		}

		m_SceneEntities.push_back(newEntity);
		return newEntity;
	}

	void Scene::ConstructSkyboxEntity(Skybox* skyBox)
	{
		m_SceneEntities.push_back(skyBox);
	}

	void Scene::DeleteSceneEntity(SceneEntity* sceneEntity)
	{
		delete sceneEntity;
	}

	std::vector<SceneEntity*> Scene::RetrieveSceneEntities()
	{
		return m_SceneEntities;
	}

	void Scene::ConstructDefaultScene()
	{
		//To implement if we want default scenes. For future scene swapping support?
	}
}