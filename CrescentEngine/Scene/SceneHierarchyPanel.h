#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <optional>

/*
	This is our scene hierarchy (UI) that is at all times linked to an ongoing active scene and a window/file context.
*/

namespace Crescent
{
	class Window;
	class Scene;
	class SceneEntity;

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(Scene* sceneContext, Window* windowContext);

		//Renders all invidual entities of the scene with UI.
		void RenderSceneEditorUI();

	private:
		void DrawEntityUI(SceneEntity* sceneEntity);
		void DrawSelectedEntityComponents(SceneEntity* selectedEntity);
		void DrawSelectedEntityMaterialSettings(SceneEntity* selectedEntity);
		void DrawVector3Controls(const std::string& uiLabel, glm::vec3& values, SceneEntity* selectedEntity, float resetValue = 0.0f, float columnWidth = 100.0f);
		void DrawSelectedEntityMaterialTextureComponent(SceneEntity* selectedEntity, const std::string& nodeName, const std::string& uniformTextureName, int uniformTextureUnit);
		void DrawSelectedEntityAnimationSettings(SceneEntity* selectedEntity);

		std::optional<std::string> OpenFile(const char* filter);

	private:
		SceneEntity* m_CurrentlySelectedEntity = nullptr;
		Scene* m_SceneContext;
		Window* m_WindowContext;
	};
}