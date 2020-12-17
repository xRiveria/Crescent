#include "CrescentPCH.h"
#include "Window.h"
#include "Editor.h"
#include "Models/Model.h"
#include "Renderer/Renderer.h"
#include "Utilities/Cubemap.h"

struct CoreSystems
{
	CrescentEngine::Window m_Window; //Setups our Window.
	CrescentEngine::Editor m_Editor; //Setups our ImGui context.
	CrescentEngine::Renderer m_Renderer; //Setups our OpenGL context.
};

struct RenderingComponents
{
	CrescentEngine::Cubemap m_Cubemap;
};

struct Renderables
{
	CrescentEngine::Model m_BackpackModel;
};

//Input Callbacks

void FramebufferResizeCallback(GLFWwindow* window, int windowWidth, int windowHeight);
void CameraAllowEulerCallback(GLFWwindow* window, int button, int action, int mods);
void CameraMovementCallback(GLFWwindow* window, double xPos, double yPos);
void CameraZoomCallback(GLFWwindow* window, double xOffset, double yOffset);

int main()
{
	//Creates our core engine systems.
	CoreSystems g_CoreSystems;

	//Initializes GLFW.
	g_CoreSystems.m_Window.CreateWindow("Crescent Engine", 1280.0f, 720.0f);

	//Callbacks
	g_CoreSystems.m_Window.SetFramebufferCallback(FramebufferResizeCallback);
	g_CoreSystems.m_Window.SetMouseCursorCallback(CameraMovementCallback);
	g_CoreSystems.m_Window.SetMouseButtonCallback(CameraAllowEulerCallback);
	g_CoreSystems.m_Window.SetMouseScrollCallback(CameraZoomCallback);

	//Setups ImGui
	g_CoreSystems.m_Editor.SetApplicationContext(g_CoreSystems.m_Window.RetrieveWindow());
	g_CoreSystems.m_Editor.InitializeImGui();

	//Initializes OpenGL.
	g_CoreSystems.m_Renderer.InitializeOpenGL();
	g_CoreSystems.m_Renderer.ToggleDepthTesting(true);


}