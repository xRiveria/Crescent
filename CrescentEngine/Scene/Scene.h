#pragma once
#include <vector>

/*
	- This is our global scene object. There will always one global scene object which can be cleared and configured at will.
*/

namespace Crescent
{
	class SceneEntity;
	class Model;
	class Material;

	class Scene
	{
	public:
		Scene(bool isEmptyScene = true);

		//Clears all scene entities currently part of the open scene.
		void ClearScene();

		//Constructs an empty scene entity. 
		SceneEntity* ConstructNewEntity();
		//Directly constructs a node with an attached Mesh and Material.
		SceneEntity* ConstructNewEntity(Model* mesh, Material* material);
		//Deletes a scene node from the global scene hierarchy together with its children.
		void DeleteSceneEntity(SceneEntity* sceneEntity);

		std::vector<SceneEntity*> RetrieveSceneEntities();
	
	private:
		void ConstructDefaultScene();

	private:
		unsigned int m_SceneEntityCounterID;
		//Cache all scene entities part of the current scene.
		std::vector<SceneEntity*> m_SceneEntities;
	};
}