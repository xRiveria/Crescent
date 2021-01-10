#include "CrescentPCH.h"
#include "Window.h"
#include "Editor.h"
#include "Shading/Texture.h"
#include "Utilities/Timestep.h"
#include "Utilities/Camera.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderQueue.h"
#include <imgui/imgui.h>
#include "../Scene/Scene.h"
#include "../Scene/SceneEntity.h"
#include "../Scene/SceneHierarchyPanel.h"
#include "Shading/Material.h"
#include "Rendering/GLStateCache.h"
#include "Rendering/RendererSettingsPanel.h"
#include "Models/DefaultPrimitives.h"
#include "Rendering/RenderTarget.h"
#include "Lighting/DirectionalLight.h"
#include "Lighting/PointLight.h"
#include <glm/gtc/type_ptr.hpp>

struct CoreSystems
{
	Crescent::Window m_Window; //Setups our Window.
	Crescent::Editor m_Editor; //Setups our ImGui context.
	Crescent::Renderer* m_Renderer; //Setups our OpenGL context.
	Crescent::Timestep m_Timestep; //Setups our Timestep.
	Crescent::Camera m_Camera = { glm::vec3(0.0f, 0.0f, 5.0) }; //Setups our Camera.

	float m_LastFrameTime = 0.0f;
};

//Our Systems	
CoreSystems g_CoreSystems; //Creates our core engine systems.

//Temporary
glm::vec3 lightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 pointLightPosition = glm::vec3(0.0f, 0.0f, 0.0f);

//Input Callbacks
void RenderEditor(Crescent::SceneHierarchyPanel* sceneHierarchyPanel, Crescent::RendererSettingsPanel* rendererPanel);
void ProcessKeyboardEvents(GLFWwindow* window);
void FramebufferResizeCallback(GLFWwindow* window, int windowWidth, int windowHeight);
void CameraAllowEulerCallback(GLFWwindow* window, int button, int action, int mods);
void CameraMovementCallback(GLFWwindow* window, double xPos, double yPos);
void CameraZoomCallback(GLFWwindow* window, double xOffset, double yOffset);

int main(int argc, int argv[])
{
	glfwWindowHint(GLFW_SAMPLES, 16);

	//Initializes GLFW.
	g_CoreSystems.m_Window.CreateNewWindow("Crescent Engine", 1280.0f, 720.0f);

	//Callbacks
	g_CoreSystems.m_Window.SetFramebufferCallback(FramebufferResizeCallback);
	g_CoreSystems.m_Window.SetMouseCursorCallback(CameraMovementCallback);
	g_CoreSystems.m_Window.SetMouseButtonCallback(CameraAllowEulerCallback);
	g_CoreSystems.m_Window.SetMouseScrollCallback(CameraZoomCallback);

	//Initializes OpenGL.
	g_CoreSystems.m_Renderer = new Crescent::Renderer();
	g_CoreSystems.m_Renderer->InitializeRenderer(1280.0f, 720.0f);
	g_CoreSystems.m_Renderer->SetSceneCamera(&g_CoreSystems.m_Camera);

	//Setups ImGui
	g_CoreSystems.m_Editor.SetApplicationContext(&g_CoreSystems.m_Window);
	g_CoreSystems.m_Editor.InitializeImGui();

	Crescent::Scene* demoScene = new Crescent::Scene();
	Crescent::SceneHierarchyPanel* sceneHierarchy = new Crescent::SceneHierarchyPanel(demoScene);
	Crescent::RendererSettingsPanel* rendererSettingsPanel = new Crescent::RendererSettingsPanel(g_CoreSystems.m_Renderer);

	//===========================================
	/// Create Default Material Here
	Crescent::Material* defaultMaterial = g_CoreSystems.m_Renderer->CreateMaterial();

	Crescent::Cube* cube = new Crescent::Cube();

	Crescent::SceneEntity* sceneCube = demoScene->ConstructNewEntity(cube, defaultMaterial);
	Crescent::SceneEntity* sceneCube2 = demoScene->ConstructNewEntity(cube, defaultMaterial);
	sceneCube2->SetEntityPosition(glm::vec3(-0.40, -1.20, 0.50));

	/// To Do: Convert directional light into a screen entity so it may be used in the scene hierarchy.
	Crescent::DirectionalLight directionalLight;
	directionalLight.m_LightDirection = lightDirection;
	directionalLight.m_LightColor = glm::vec3(1.0f, 0.89f, 0.7f);
	directionalLight.m_LightIntensity = 50.0f;

	Crescent::PointLight pointLight;
	pointLight.m_LightRadius = 2.5f;
	pointLight.m_LightColor = glm::vec3(1.0f, 0.3f, 0.05f);
	pointLight.m_LightIntensity = 50.0f;
	pointLight.m_RenderMesh = true;

	pointLight.m_LightPosition = pointLightPosition;

	g_CoreSystems.m_Renderer->AddLightSource(&pointLight);
	g_CoreSystems.m_Renderer->AddLightSource(&directionalLight);
	//===========================================

	while (!g_CoreSystems.m_Window.RetrieveWindowCloseStatus())
	{
		//Check Projection Matrix
		if (g_CoreSystems.m_Editor.RetrieveViewportWidth() > 0.0f && g_CoreSystems.m_Editor.RetrieveViewportHeight() > 0.0f && (g_CoreSystems.m_Renderer->RetrieveRenderWindowSize().x != g_CoreSystems.m_Editor.RetrieveViewportWidth() || g_CoreSystems.m_Renderer->RetrieveRenderWindowSize().y != g_CoreSystems.m_Editor.RetrieveViewportHeight()))
		{
			g_CoreSystems.m_Renderer->SetRenderingWindowSize(g_CoreSystems.m_Editor.RetrieveViewportWidth(), g_CoreSystems.m_Editor.RetrieveViewportHeight());
			CrescentInfo("Resized Render Target!");
		}
		g_CoreSystems.m_Camera.m_ProjectionMatrix = glm::perspective(glm::radians(g_CoreSystems.m_Camera.m_MouseZoom), ((float)g_CoreSystems.m_Editor.RetrieveViewportWidth() / (float)g_CoreSystems.m_Editor.RetrieveViewportHeight()), 0.2f, 100.0f);

		//Retrieve Delta Time
		float currentFrame = g_CoreSystems.m_Window.RetrieveCurrentTime();
		g_CoreSystems.m_Timestep = currentFrame - g_CoreSystems.m_LastFrameTime;
		g_CoreSystems.m_LastFrameTime = currentFrame;

		//Poll Events
		g_CoreSystems.m_Window.PollEvents();
		ProcessKeyboardEvents(g_CoreSystems.m_Window.RetrieveWindow());
	
		g_CoreSystems.m_Camera.UpdateCameraVectors();

		//Randomize
		pointLight.m_LightRadius = 1.5f + 0.1 * std::cos(std::sin(glfwGetTime() * 1.37 + 0 * 7.31) * 3.1 + 0);
		pointLight.m_LightIntensity = 25.0f + 5.0 * std::cos(std::sin(glfwGetTime() * 0.67 + 0 * 2.31) * 2.31 * 0);

		//Rendering
		g_CoreSystems.m_Renderer->PushToRenderQueue(sceneCube);
		g_CoreSystems.m_Renderer->PushToRenderQueue(sceneCube2);

		g_CoreSystems.m_Renderer->RenderAllQueueItems();

		//We reset the framebuffer back to normal here for our Editor.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderEditor(sceneHierarchy, rendererSettingsPanel);

		g_CoreSystems.m_Window.SwapBuffers();
	}

	g_CoreSystems.m_Window.TerminateWindow();
	return 0;
}

void RenderEditor(Crescent::SceneHierarchyPanel* sceneHierarchyPanel, Crescent::RendererSettingsPanel* rendererPanel)
{
	g_CoreSystems.m_Editor.BeginEditorRenderLoop();
	g_CoreSystems.m_Editor.RenderDockingContext(); //This contains a Begin().

	ImGui::Begin("Lighting - Temporary");
	ImGui::DragFloat3("Point Light Position 1", glm::value_ptr(pointLightPosition), 0.10f);
	ImGui::DragFloat3("Light Direction", glm::value_ptr(lightDirection), 0.10f);
	ImGui::End();

	sceneHierarchyPanel->RenderSceneEditorUI();
	rendererPanel->RenderRendererEditorUI();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

	ImGui::Begin("Deferred Rendering");
	for (int i = 0; i < 4; i++)
	{
		unsigned int colorAttachment = g_CoreSystems.m_Renderer->RetrieveGBuffer()->RetrieveColorAttachment(i)->RetrieveTextureID();
		ImGui::Image((void*)colorAttachment, { 350.0f, 350.0f }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		if (i % 2 == 0)
		{
			ImGui::SameLine();
		}
	}

	ImGui::Text("Shadow Map #1");
	unsigned int shadowDepthAttachment = g_CoreSystems.m_Renderer->RetrieveShadowRenderTarget(0)->RetrieveDepthAndStencilAttachment()->RetrieveTextureID();
	ImGui::Image((void*)shadowDepthAttachment, { 350.0f, 350.0f }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

	ImGui::Text("Custom Render Target Color Buffer");
	ImGui::Image((void*)g_CoreSystems.m_Renderer->RetrieveCustomRenderTarget()->RetrieveColorAttachment(0)->RetrieveTextureID(), { 350.0f, 350.0f }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
	ImGui::Text("Custom Render Target Depth/Stencil Buffer");
	ImGui::Image((void*)g_CoreSystems.m_Renderer->RetrieveCustomRenderTarget()->RetrieveDepthAndStencilAttachment()->RetrieveTextureID(), { 350.0f, 350.0f }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

	ImGui::End();

	ImGui::Begin("Viewport");
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	g_CoreSystems.m_Editor.SetViewportSize(viewportPanelSize.x, viewportPanelSize.y); //The current size of our viewport.

	unsigned int colorAttachment = g_CoreSystems.m_Renderer->RetrieveMainRenderTarget()->RetrieveColorAttachment(0)->RetrieveTextureID();
	ImGui::Image((void*)colorAttachment, { (float)g_CoreSystems.m_Editor.RetrieveViewportWidth(), (float)g_CoreSystems.m_Editor.RetrieveViewportHeight() }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
	
	ImGui::End();
	ImGui::PopStyleVar(); //Pops the pushed style so other windows beyond this won't have the style's properties.

	ImGui::End(); //Closes the docking context.
	g_CoreSystems.m_Editor.EndEditorRenderLoop();
}

//Event Callbacks - To Be Further Abstracted.
void ProcessKeyboardEvents(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		g_CoreSystems.m_Window.TerminateWindow();
	}

	const float cameraSpeed = g_CoreSystems.m_Timestep.GetDeltaTimeInSeconds() * 2.5f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_CoreSystems.m_Camera.ProcessKeyboardEvents(Crescent::CameraMovement::Forward, cameraSpeed);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_CoreSystems.m_Camera.ProcessKeyboardEvents(Crescent::CameraMovement::Backward, cameraSpeed);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_CoreSystems.m_Camera.ProcessKeyboardEvents(Crescent::CameraMovement::Left, cameraSpeed);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_CoreSystems.m_Camera.ProcessKeyboardEvents(Crescent::CameraMovement::Right, cameraSpeed);
	}
}

void CameraAllowEulerCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		Crescent::g_CameraMode = true;
	}
	else
	{
		Crescent::g_CameraMode = false;
	}
}

void CameraMovementCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (Crescent::g_IsCameraFirstMove)
	{
		Crescent::g_CameraLastXPosition = xPos;
		Crescent::g_CameraLastYPosition = yPos;
		Crescent::g_IsCameraFirstMove = false;
	}

	float xOffset = xPos - Crescent::g_CameraLastXPosition;
	float yOffset = Crescent::g_CameraLastYPosition - yPos; //Reversed since Y Coordinates go from bottom to top.
	Crescent::g_CameraLastXPosition = xPos;

	Crescent::g_CameraLastYPosition = yPos;

	if (Crescent::g_CameraMode)
	{
		g_CoreSystems.m_Camera.ProcessMouseMovement(xOffset, yOffset);
	}
}

void CameraZoomCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	g_CoreSystems.m_Camera.ProcessMouseScroll(yOffset);
}

//This is a callback function that is called whenever a window is resized.
void FramebufferResizeCallback(GLFWwindow* window, int windowWidth, int windowHeight)
{
	//g_CoreSystems.m_Renderer->SetRenderingWindowSize(windowWidth, windowHeight);
	g_CoreSystems.m_Window.ResizeWindow((float)windowWidth, (float)windowHeight);
}


