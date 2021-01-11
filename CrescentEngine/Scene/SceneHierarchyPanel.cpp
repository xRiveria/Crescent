#include "CrescentPCH.h"
#include "SceneHierarchyPanel.h"
#include "Scene.h"
#include "SceneEntity.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Crescent
{
    SceneHierarchyPanel::SceneHierarchyPanel(Scene* sceneContext)
    {
        m_SceneContext = sceneContext;
        m_CurrentlySelectedEntity = nullptr;
    }

	void SceneHierarchyPanel::RenderSceneEditorUI()  //Flows information into the other UI render functions.
	{
		//Main Scene UI.
		ImGui::Begin("Hierarchy");

		for (int i = 0; i < m_SceneContext->RetrieveSceneEntities().size(); i++)
		{
			DrawEntityUI(m_SceneContext->RetrieveSceneEntities()[i]);
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0)) //If we click elsewhere in the window, we deselect the current object.
		{
			m_CurrentlySelectedEntity = nullptr;
		}

		//Right clicking on an empty space within the scene's hierarchy.
		if (ImGui::BeginPopupContextWindow(0, 1, false)) //Last parameter makes it so that we cannot right click and openup the popup whilst hovering over items.
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				m_SceneContext->ConstructNewEntity();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		//Per Entity UI
		ImGui::Begin("Properties");
		if (m_CurrentlySelectedEntity != nullptr)
		{
			DrawSelectedEntityComponents(m_CurrentlySelectedEntity);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityUI(SceneEntity* sceneEntity)
	{
		ImGuiTreeNodeFlags flags = ((m_CurrentlySelectedEntity == sceneEntity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool isExpanded = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)*sceneEntity, flags, sceneEntity->RetrieveEntityName().c_str());

		if (ImGui::IsItemClicked())
		{
			m_CurrentlySelectedEntity = sceneEntity;
		}

		if (isExpanded)
		{
			ImGui::TreePop(); //Pop it for now since we don't have child entity support yet.
		}
	}

	void SceneHierarchyPanel::DrawSelectedEntityComponents(SceneEntity* selectedEntity)
	{
		char nameBuffer[256];
		memset(nameBuffer, 0, sizeof(nameBuffer));
		strcpy_s(nameBuffer, sizeof(nameBuffer), selectedEntity->RetrieveEntityName().c_str());

		if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer)))
		{
			selectedEntity->SetEntityName(std::string(nameBuffer));
		}

		DrawVector3Controls("Translation", selectedEntity->RetrieveEntityPosition(), selectedEntity);

		//Converts our radians to degrees for display in the hierarchy, and translates its new values back to radians.
		glm::vec3 rotation = (glm::vec3)glm::degrees(selectedEntity->RetrieveEntityRotation());
		DrawVector3Controls("Rotation", rotation, selectedEntity);
		selectedEntity->SetEntityRotation(glm::radians(rotation));

		DrawVector3Controls("Scale", selectedEntity->RetrieveEntityScale(), selectedEntity);
	}

	void SceneHierarchyPanel::DrawVector3Controls(const std::string& uiLabel, glm::vec3& values, SceneEntity* selectedEntity, float resetValue, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(uiLabel.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth); //First Column
		ImGui::Text(uiLabel.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);

		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
			selectedEntity->UpdateEntityTransform(true);
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
		{
			selectedEntity->UpdateEntityTransform(true);
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);

		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
			selectedEntity->UpdateEntityTransform(true);
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
		{
			selectedEntity->UpdateEntityTransform(true);
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);

		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
			selectedEntity->UpdateEntityTransform(true);
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
		{
			selectedEntity->UpdateEntityTransform(true);
		}
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}
}