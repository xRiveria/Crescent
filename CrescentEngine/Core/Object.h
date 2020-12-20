#pragma once
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

//Create Base Entity Class Please.

namespace CrescentEngine
{
	struct DirectionalLight
	{
		DirectionalLight() {}

		glm::vec3 lightDirection = { -0.2f, -1.0f, -0.3f };
		glm::vec3 ambientIntensity = { 0.2f, 0.2f, 0.2f };
		glm::vec3 diffuseIntensity = { 0.5f, 0.5f, 0.5f };
		glm::vec3 specularIntensity = { 1.0f, 1.0f, 1.0f };

		void RenderSettingsInEditor()
		{
			ImGui::Begin("Directional Light");
			//ImGui::DragFloat("Specular Highlight", &g_SpecularScattering, 0.2f, 2.0f, 256.0f);
			ImGui::DragFloat3("Light Direction", glm::value_ptr(lightDirection), 0.1f);
			ImGui::DragFloat3("Ambient Intensity##Direction", glm::value_ptr(ambientIntensity), 0.1f);
			ImGui::DragFloat3("Diffuse Intensity##Direction", glm::value_ptr(diffuseIntensity), 0.1f);
			ImGui::DragFloat3("Specular Intensity##Direction", glm::value_ptr(specularIntensity), 0.1f);
			ImGui::End();
		}
	};

	struct PointLight
	{
		PointLight() {}

		glm::vec3 pointLightPosition = { 0.7f, 0.2f, 2.0f };
		glm::vec3 ambientIntensity = { 0.2f, 0.2f, 0.2f };
		glm::vec3 diffuseIntensity = { 0.5f, 0.5f, 0.5f };
		glm::vec3 specularIntensity = { 1.0f, 1.0f, 1.0f };

		void RenderSettingsInEditor()
		{
			ImGui::Begin("Point Light");
			ImGui::DragFloat3("Light Position", glm::value_ptr(pointLightPosition), 0.1f);
			ImGui::DragFloat3("Ambient Intensity##Point", glm::value_ptr(ambientIntensity), 0.1f);
			ImGui::DragFloat3("Diffuse Intensity##Point", glm::value_ptr(diffuseIntensity), 0.1f);
			ImGui::DragFloat3("Specular Intensity##Point", glm::value_ptr(specularIntensity), 0.1f);
			ImGui::End();
		}
	};
}