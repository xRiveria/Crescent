#include "CrescentPCH.h"
#include "SceneHierarchyPanel.h"
#include "Scene.h"
#include "../Core/Window.h"
#include "SceneEntity.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "../Shading/Texture.h"
#include "../Shading/Material.h"
#include "../Rendering/Resources.h"
#include "../Models/Mesh.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> //Allows us to retrieve the Window handle.

namespace Crescent
{
    SceneHierarchyPanel::SceneHierarchyPanel(Scene* sceneContext, Window* windowContext) : m_SceneContext(sceneContext), m_WindowContext(windowContext)
    {	
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

		//Material
		ImGui::Begin("Material");
		if (m_CurrentlySelectedEntity != nullptr)
		{
			DrawSelectedEntityMaterialSettings(m_CurrentlySelectedEntity);
		}
		ImGui::End();

		//Animation
		ImGui::Begin("Animations");
		if (m_CurrentlySelectedEntity != nullptr)
		{
			DrawSelectedEntityAnimationSettings(m_CurrentlySelectedEntity);
		}
	}

	static MeshAnimation* currentSelectedAnimation = nullptr;

	void SceneHierarchyPanel::DrawSelectedEntityAnimationSettings(SceneEntity* selectedEntity)
	{
		if (selectedEntity->m_Mesh != nullptr)
		{
			if (!selectedEntity->m_Mesh->m_Animations.empty())
			{
				bool replayAnimation = true;
				ImGui::Button("Play");
				ImGui::SameLine();
				ImGui::Button("Pause");
				ImGui::SameLine();
				ImGui::Checkbox("Loop", &replayAnimation);

				std::vector<MeshAnimation*> meshAnimations = selectedEntity->m_Mesh->m_Animations;
				if (currentSelectedAnimation == nullptr)
				{
					currentSelectedAnimation = meshAnimations[0];
				}
				//List all animations.
				ImGui::Spacing();
				if (ImGui::BeginCombo("##Animations", currentSelectedAnimation->m_AnimationName.c_str()))
				{
					for (int i = 0; i < meshAnimations.size(); i++)
					{
						bool isSelected = (currentSelectedAnimation == meshAnimations[i]);
						if (ImGui::Selectable(meshAnimations[i]->m_AnimationName.c_str(), isSelected))
						{
							currentSelectedAnimation = meshAnimations[i];
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGui::Spacing();
				//Timer for each selected animation.
				ImGui::SliderFloat("##Timer", &currentSelectedAnimation->m_AnimationTimeInSeconds, 0.0f, meshAnimations[0]->m_AnimationTimeInSeconds, "%.2f", ImGuiTreeNodeFlags_SpanAvailWidth);
			}
		}
	}

	void SceneHierarchyPanel::DrawSelectedEntityMaterialSettings(SceneEntity* selectedEntity)
	{
		if (selectedEntity->m_Material != nullptr)
		{
			DrawSelectedEntityMaterialTextureComponent(selectedEntity, "Albedo", "TexAlbedo", 3);
			DrawSelectedEntityMaterialTextureComponent(selectedEntity, "Normal", "TexNormal", 4);
			DrawSelectedEntityMaterialTextureComponent(selectedEntity, "Metallic", "TexMetallic", 5);
			DrawSelectedEntityMaterialTextureComponent(selectedEntity, "Roughness", "TexRoughness", 6);
			DrawSelectedEntityMaterialTextureComponent(selectedEntity, "Ambient Occlusion", "TexAO", 7);
		}
	}

	std::optional<std::string> SceneHierarchyPanel::OpenFile(const char* filter)
	{
		OPENFILENAMEA fileDialog; //Passes data to and from GetOpenFileName & GetSaveFileName. It stores settings used to create the dialog box and the results of the user's selection. 
		CHAR szFile[260] = { 0 }; //Our selected file path's buffer.
		ZeroMemory(&fileDialog, sizeof(OPENFILENAME)); //Initialize openedFile's memory to 0.

		fileDialog.lStructSize = sizeof(OPENFILENAME); //Sets the struct size. We do this for every Win32 struct.
		fileDialog.hwndOwner = glfwGetWin32Window(m_WindowContext->RetrieveWindow()); //Gets our currently open window and retrieves it HWND which we set as the struct's owner.
		fileDialog.lpstrFile = szFile; //Buffer for our file.
		fileDialog.nMaxFile = sizeof(szFile); //Size of our file buffer.
		fileDialog.lpstrFilter = filter; //File filter.
		fileDialog.nFilterIndex = 1; //Which filter is set by default. 
		fileDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; //The last flag is very important. If you don't do this and call OpenFileName, it will change the working directory for your application to the folder you open the window from.  

		if (GetOpenFileNameA(&fileDialog) == true)
		{
			return fileDialog.lpstrFile; //We return the file path of the file we open and create a string out of the char* path.
		}

		return std::nullopt; //Return empty string if no file is selected. It means the dialog has been cancelled.
	}

	void SceneHierarchyPanel::DrawSelectedEntityMaterialTextureComponent(SceneEntity* selectedEntity, const std::string& nodeName, const std::string& uniformTextureName, int uniformTextureUnit)
	{
		if (selectedEntity->m_Material->m_SamplerUniforms[uniformTextureName].m_Texture != nullptr)
		{
			if (ImGui::CollapsingHeader(nodeName.c_str()))
			{
				ImGui::Spacing();
				ImGui::Image((void*)selectedEntity->m_Material->m_SamplerUniforms[uniformTextureName].m_Texture->RetrieveTextureID(), { 100.0f, 100.0f }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
				ImGui::SameLine();
				if (ImGui::Button("Load New Texture"))
				{
					std::optional<std::string> filePath = OpenFile("Textures");
					if (filePath.has_value())
					{
						std::string path = filePath.value();
						std::string filePath = path.substr(0, path.find_last_of("/"));

						Texture* loadedTexture = Resources::LoadTexture(filePath, filePath, GL_TEXTURE_2D, GL_RGB, true);
						selectedEntity->m_Material->SetShaderTexture(uniformTextureName, loadedTexture, uniformTextureUnit);				
					}
				}

				ImGui::Spacing();
			}
		}
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
			for (int i = 0; i < sceneEntity->m_ChildEntities.size(); i++)
			{
				SceneEntity* childEntity = sceneEntity->m_ChildEntities[i];
				if (childEntity->m_Material != nullptr)
				{
					DrawEntityUI(childEntity);
				}
			}
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