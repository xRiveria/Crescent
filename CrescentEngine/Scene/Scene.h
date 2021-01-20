#pragma once
#include <vector>

/*
	- This is our global scene object. There will always one global scene object which can be cleared and configured at will.
*/

namespace Crescent
{
	class SceneEntity;
	class Model;
	class Mesh;
	class Material;
	class PointLight;
	class DirectionalLight;
	class Skybox;

	class Scene
	{
	public:
		Scene(bool isEmptyScene = true);

		//Clears all scene entities currently part of the open scene.
		void ClearScene();

		//Constructs an empty scene entity. 
		SceneEntity* ConstructNewEntity();
		//Directly constructs a node with an attached Mesh and Material.
		SceneEntity* ConstructNewEntity(Mesh* mesh, Material* material);
		//Directly constructs a node for lighting purposes.
		SceneEntity* ConstructNewEntity(PointLight* pointLight); ///Take lighting positions from the scene entity.
		SceneEntity* ConstructNewEntity(DirectionalLight* directionalLight); ///Take lighting rotations from the scene entity.

		SceneEntity* ConstructNewEntity(SceneEntity* sceneEntity);

		void ConstructSkyboxEntity(Skybox* skyBox);

		//Deletes a scene node from the global scene hierarchy together with its children.
		void DeleteSceneEntity(SceneEntity* sceneEntity);

		std::vector<SceneEntity*> RetrieveSceneEntities();

	public:
		static unsigned int m_SceneEntityCounterID;
	
	private:
		void ConstructDefaultScene();

	private:
		//Cache all scene entities part of the current scene.
		std::vector<SceneEntity*> m_SceneEntities;
	};
}