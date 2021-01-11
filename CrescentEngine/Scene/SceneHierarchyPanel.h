#pragma once
#include <vector>
#include <glm/glm.hpp>

/*
	This is our scene hierarchy (UI) that is at all times linked to an ongoing active scene.
*/

namespace Crescent
{
	class Scene;
	class SceneEntity;

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(Scene* sceneContext);

		//Renders all invidual entities of the scene with UI.
		void RenderSceneEditorUI();

	private:
		void DrawEntityUI(SceneEntity* sceneEntity);
		void DrawSelectedEntityComponents(SceneEntity* selectedEntity);
		void DrawVector3Controls(const std::string& uiLabel, glm::vec3& values, SceneEntity* selectedEntity, float resetValue = 0.0f, float columnWidth = 100.0f);

	private:
		SceneEntity* m_CurrentlySelectedEntity = nullptr;
		Scene* m_SceneContext;
	};
}