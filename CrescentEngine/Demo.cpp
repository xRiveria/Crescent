#include "CrescentPCH.h"
#include "Window.h"
#include "Editor.h"
#include "Shading/Texture.h"
#include "Utilities/Timestep.h"
#include "Utilities/Camera.h"
#include "Rendering/Framebuffer.h";
#include "Models/Model.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderQueue.h"
#include "Object.h"
#include "Rendering/Cubemap.h"
#include <stb_image/stb_image.h>
#include <imgui/imgui.h>
#include "../Scene/Scene.h"
#include "../Scene/SceneEntity.h"
#include "../Scene/SceneHierarchyPanel.h"
#include "Shading/Material.h"
#include "Rendering/GLStateCache.h"
#include "Rendering/RendererSettingsPanel.h"
#include "Models/DefaultPrimitives.h"
#include "Rendering/RenderTarget.h"

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
	//===========================================

	while (!g_CoreSystems.m_Window.RetrieveWindowCloseStatus())
	{
		glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
		//Check Projection Matrix

		//Check Projection Matrix
		if (g_CoreSystems.m_Editor.RetrieveViewportWidth() > 0.0f && g_CoreSystems.m_Editor.RetrieveViewportHeight() > 0.0f && (g_CoreSystems.m_Renderer->RetrieveRenderWindowSize().x != g_CoreSystems.m_Editor.RetrieveViewportWidth() || g_CoreSystems.m_Renderer->RetrieveRenderWindowSize().y != g_CoreSystems.m_Editor.RetrieveViewportHeight()))
		{
			g_CoreSystems.m_Renderer->SetRenderingWindowSize(g_CoreSystems.m_Editor.RetrieveViewportWidth(), g_CoreSystems.m_Editor.RetrieveViewportHeight());
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

		glBindFramebuffer(GL_FRAMEBUFFER, g_CoreSystems.m_Renderer->RetrieveMainRenderTarget()->m_FramebufferID);
		//Rendering
		g_CoreSystems.m_Renderer->PushToRenderQueue(sceneCube);
		g_CoreSystems.m_Renderer->RenderAllQueueItems();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//We reset the framebuffer back to normal here for our Editor.
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

	sceneHierarchyPanel->RenderSceneEditorUI();
	rendererPanel->RenderRendererEditorUI();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

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


