#include "CrescentPCH.h"
#include "Scene.h"
#include "SceneEntity.h"

namespace Crescent
{
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
		SceneEntity* newEntity = new SceneEntity("Empty Entity", m_SceneEntityCounterID++);
		m_SceneEntities.push_back(newEntity);

		return newEntity;
	}

	SceneEntity* Scene::ConstructNewEntity(Mesh* mesh, Material* material)
	{
		SceneEntity* newEntity = new SceneEntity("Model", m_SceneEntityCounterID++);
		
		newEntity->m_Mesh = mesh;
		newEntity->m_Material = material;

		m_SceneEntities.push_back(newEntity);
		
		return newEntity;
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