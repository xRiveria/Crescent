#include "CrescentPCH.h"
#include "Window.h"
#include "Editor.h"
#include "Utilities/Timestep.h"
#include "Utilities/Camera.h"
#include "Rendering/Framebuffer.h";
#include "Models/Model.h"
#include "Rendering/Renderer.h"
#include "Object.h"
#include "Rendering/Cubemap.h"
#include <stb_image/stb_image.h>
#include <imgui/imgui.h>

struct CoreSystems
{
	CrescentEngine::Window m_Window; //Setups our Window.
	CrescentEngine::Editor m_Editor; //Setups our ImGui context.
	CrescentEngine::Renderer m_Renderer; //Setups our OpenGL context.
	CrescentEngine::Timestep m_Timestep; //Setups our Timestep.
	CrescentEngine::Camera m_Camera = { glm::vec3(0.0f, 0.0f, 3.0f) }; //Setups our Camera.
	float m_LastFrameTime = 0.0f;
};

struct RenderingComponents
{
	CrescentEngine::Framebuffer m_Framebuffer;
	CrescentEngine::Cubemap m_Cubemap;

	bool m_WireframeRendering = false;
	bool m_OutlineRendering = false;
};

struct Renderables
{
	CrescentEngine::Model m_BackpackModel;
	CrescentEngine::Model m_RedstoneLampModel;

	CrescentEngine::DirectionalLight m_LightDirection;
	CrescentEngine::PointLight m_PointLight;

	//Cubemap
	std::vector<std::string> m_OceanCubemap = {
		"Resources/Skybox/Ocean/right.jpg",
		"Resources/Skybox/Ocean/left.jpg",
		"Resources/Skybox/Ocean/top.jpg",
		"Resources/Skybox/Ocean/bottom.jpg",
		"Resources/Skybox/Ocean/front.jpg",
		"Resources/Skybox/Ocean/back.jpg"
	};
};

struct Shaders
{
	CrescentEngine::LearnShader m_StaticModelShader;
	CrescentEngine::LearnShader m_PointLightObjectShader;
	CrescentEngine::LearnShader m_OutlineObjectShader;
};

//Our Systems	
CoreSystems g_CoreSystems; //Creates our core engine systems.
RenderingComponents g_RenderingComponents; //Creates our rendering components.
Renderables g_Renderables; //Creates our assets.
Shaders g_Shaders; //Creates our shaders.

//Input Callbacks
void ProcessKeyboardEvents(GLFWwindow* window);
void FramebufferResizeCallback(GLFWwindow* window, int windowWidth, int windowHeight);
void CameraAllowEulerCallback(GLFWwindow* window, int button, int action, int mods);
void CameraMovementCallback(GLFWwindow* window, double xPos, double yPos);
void CameraZoomCallback(GLFWwindow* window, double xOffset, double yOffset);

glm::mat4 projectionMatrix = glm::mat4(1.0f);
float m_AspectRatio = 1280.0 / 1080.0;

int main(int argc, int argv[])
{
	//Initializes GLFW.
	g_CoreSystems.m_Window.CreateWindow("Crescent Engine", 1280.0f, 720.0f);

	//Callbacks
	g_CoreSystems.m_Window.SetFramebufferCallback(FramebufferResizeCallback);
	g_CoreSystems.m_Window.SetMouseCursorCallback(CameraMovementCallback);
	g_CoreSystems.m_Window.SetMouseButtonCallback(CameraAllowEulerCallback);
	g_CoreSystems.m_Window.SetMouseScrollCallback(CameraZoomCallback);

	//Initializes OpenGL.
	g_CoreSystems.m_Renderer.InitializeOpenGL();
	g_CoreSystems.m_Renderer.ToggleDepthTesting(true);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//Setups ImGui
	g_CoreSystems.m_Editor.SetApplicationContext(&g_CoreSystems.m_Window);
	g_CoreSystems.m_Editor.InitializeImGui();

	g_RenderingComponents.m_Framebuffer.InitializeFramebuffer(g_CoreSystems.m_Window.RetrieveWindowWidth(), g_CoreSystems.m_Window.RetrieveWindowHeight());

	//Cubemaps
	g_RenderingComponents.m_Cubemap.LoadCubemap(g_Renderables.m_OceanCubemap);
	g_RenderingComponents.m_Cubemap.SetupCubemapBuffers();

	//Shaders
	g_Shaders.m_StaticModelShader.CreateShaders("Resources/Shaders/StaticModelVertex.shader", "Resources/Shaders/StaticModelFragment.shader");
	g_Shaders.m_PointLightObjectShader.CreateShaders("Resources/Shaders/LightVertexShader.shader", "Resources/Shaders/LightFragmentShader.shader");
	g_Shaders.m_OutlineObjectShader.CreateShaders("Resources/Shaders/OutlineVertex.shader", "Resources/Shaders/OutlineFragment.shader");

	//Models
	stbi_set_flip_vertically_on_load(true);
	g_Renderables.m_BackpackModel.LoadModel("Resources/Models/Backpack/backpack.obj");
	g_Renderables.m_RedstoneLampModel.LoadModel("Resources/Models/RedstoneLamp/Redstone-lamp.obj");
	stbi_set_flip_vertically_on_load(false);

	while (!g_CoreSystems.m_Window.RetrieveWindowCloseStatus())
	{
		//Check Projection Matrix
		if (g_CoreSystems.m_Editor.RetrieveViewportWidth() > 0.0f && g_CoreSystems.m_Editor.RetrieveViewportHeight() > 0.0f && (g_RenderingComponents.m_Framebuffer.RetrieveFramebufferWidth() != g_CoreSystems.m_Editor.RetrieveViewportWidth() || g_RenderingComponents.m_Framebuffer.RetrieveFramebufferHeight() != g_CoreSystems.m_Editor.RetrieveViewportHeight()))
		{
			std::cout << "Not Correct! Updating Buffers!" << "\n";
			g_RenderingComponents.m_Framebuffer.ResizeFramebuffer(g_CoreSystems.m_Editor.RetrieveViewportWidth(), g_CoreSystems.m_Editor.RetrieveViewportHeight());
			m_AspectRatio = (float)g_CoreSystems.m_Editor.RetrieveViewportWidth() / (float)g_CoreSystems.m_Editor.RetrieveViewportHeight();
			projectionMatrix = glm::perspective(glm::radians(g_CoreSystems.m_Camera.m_MouseZoom), m_AspectRatio, 0.2f, 100.0f);
		}

		//Retrieve Delta Time
		float currentFrame = g_CoreSystems.m_Window.RetrieveCurrentTime();
		g_CoreSystems.m_Timestep = currentFrame - g_CoreSystems.m_LastFrameTime;
		g_CoreSystems.m_LastFrameTime = currentFrame;

		//Poll Events
		g_CoreSystems.m_Window.PollEvents();
		ProcessKeyboardEvents(g_CoreSystems.m_Window.RetrieveWindow());

		//Bind Our Framebuffer
		g_RenderingComponents.m_Framebuffer.BindFramebuffer();

		//Clear Buffers
		g_CoreSystems.m_Renderer.ClearBuffers();

		//View/Projection Matrix
		glm::mat4 viewMatrix = g_CoreSystems.m_Camera.GetViewMatrix();

		glStencilMask(0x00);
		//Backpack Model - To Be Further Abstracted ===========================================================================
		g_Shaders.m_StaticModelShader.UseShader();
		g_Shaders.m_StaticModelShader.SetUniformVector3("directionalLight.lightDirection", g_Renderables.m_LightDirection.lightDirection);
		g_Shaders.m_StaticModelShader.SetUniformVector3("directionalLight.ambientIntensity", g_Renderables.m_LightDirection.ambientIntensity);
		g_Shaders.m_StaticModelShader.SetUniformVector3("directionalLight.diffuseIntensity", g_Renderables.m_LightDirection.diffuseIntensity);
		g_Shaders.m_StaticModelShader.SetUniformVector3("directionalLight.specularIntensity", g_Renderables.m_LightDirection.specularIntensity);

		g_Shaders.m_StaticModelShader.SetUniformVector3("pointLight.lightPosition", g_Renderables.m_PointLight.pointLightPosition);
		g_Shaders.m_StaticModelShader.SetUniformFloat("pointLight.attenuationConstant", 1.0f);
		g_Shaders.m_StaticModelShader.SetUniformFloat("pointLight.attenuationLinear", 0.09f);
		g_Shaders.m_StaticModelShader.SetUniformFloat("pointLight.attenuationQuadratic", 0.032f);
		g_Shaders.m_StaticModelShader.SetUniformVector3("pointLight.ambientIntensity", g_Renderables.m_PointLight.ambientIntensity);
		g_Shaders.m_StaticModelShader.SetUniformVector3("pointLight.diffuseIntensity", g_Renderables.m_PointLight.diffuseIntensity);
		g_Shaders.m_StaticModelShader.SetUniformVector3("pointLight.specularIntensity", g_Renderables.m_PointLight.specularIntensity);

		g_Shaders.m_StaticModelShader.SetUniformMat4("projection", projectionMatrix);
		g_Shaders.m_StaticModelShader.SetUniformMat4("view", viewMatrix);
		g_Shaders.m_StaticModelShader.SetUniformVector3("viewPosition", g_CoreSystems.m_Camera.m_CameraPosition);
		glm::mat4 backpackModel = glm::mat4(1.0f);
		backpackModel = glm::translate(backpackModel, glm::vec3(0.0f, 0.0f, 0.0f));
		backpackModel = glm::scale(backpackModel, glm::vec3(1.0f, 1.0f, 1.0f));
		g_Shaders.m_StaticModelShader.SetUniformMat4("model", backpackModel);

		g_Renderables.m_BackpackModel.Draw(g_Shaders.m_StaticModelShader);

		//Redstone Lamp Model
		g_Shaders.m_PointLightObjectShader.UseShader();
		g_Shaders.m_PointLightObjectShader.SetUniformMat4("projection", projectionMatrix);
		g_Shaders.m_PointLightObjectShader.SetUniformMat4("view", viewMatrix);

		glm::mat4 redstoneLampModel = glm::mat4(1.0f);
		redstoneLampModel = glm::translate(redstoneLampModel, g_Renderables.m_PointLight.pointLightPosition);
		redstoneLampModel = glm::scale(redstoneLampModel, glm::vec3(0.01f)); //A smaller cube.     
		g_Shaders.m_PointLightObjectShader.SetUniformMat4("model", redstoneLampModel);

		g_Renderables.m_RedstoneLampModel.Draw(g_Shaders.m_PointLightObjectShader);

		//=======================================================================================================================

		//Calculates the front vector from the Camera's (updated) Euler Angles.
		g_CoreSystems.m_Camera.UpdateCameraVectors();

		//Draw Cubemap
		g_RenderingComponents.m_Cubemap.DrawCubemap(viewMatrix, projectionMatrix);

		g_RenderingComponents.m_Framebuffer.UnbindFramebuffer();

		//We reset the framebuffer back to normal here for ImGui to render to the default framebuffer.
		//Our ImGui Editor
		g_CoreSystems.m_Editor.BeginEditorRenderLoop();
		g_CoreSystems.m_Editor.RenderDockingContext(); //This contains a Begin().

		ImGui::Begin("Global Settings");
		if (ImGui::Checkbox("Wireframe Rendering", &g_RenderingComponents.m_WireframeRendering))
		{
			g_CoreSystems.m_Renderer.ToggleWireframeRendering(g_RenderingComponents.m_WireframeRendering);
		}
		ImGui::End();

		g_Renderables.m_LightDirection.RenderSettingsInEditor();
		g_Renderables.m_PointLight.RenderSettingsInEditor();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		g_CoreSystems.m_Editor.SetViewportSize(viewportPanelSize.x, viewportPanelSize.y); //The current size of our viewport.
		unsigned int colorAttachment = g_RenderingComponents.m_Framebuffer.RetrieveColorAttachment();
		ImGui::Image((void*)colorAttachment, { (float)g_CoreSystems.m_Editor.RetrieveViewportWidth(), (float)g_CoreSystems.m_Editor.RetrieveViewportHeight() }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		ImGui::End();
		ImGui::PopStyleVar(); //Pops the pushed style so other windows beyond this won't have the style's properties.

		ImGui::End(); //Closes the docking context.
		g_CoreSystems.m_Editor.EndEditorRenderLoop();

		g_CoreSystems.m_Window.SwapBuffers();
	}

	g_CoreSystems.m_Window.TerminateWindow();
	return 0;
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
		g_CoreSystems.m_Camera.ProcessKeyboardEvents(CrescentEngine::CameraMovement::Forward, cameraSpeed);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_CoreSystems.m_Camera.ProcessKeyboardEvents(CrescentEngine::CameraMovement::Backward, cameraSpeed);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_CoreSystems.m_Camera.ProcessKeyboardEvents(CrescentEngine::CameraMovement::Left, cameraSpeed);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_CoreSystems.m_Camera.ProcessKeyboardEvents(CrescentEngine::CameraMovement::Right, cameraSpeed);
	}
}

void CameraAllowEulerCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		CrescentEngine::g_CameraMode = true;
	}
	else
	{
		CrescentEngine::g_CameraMode = false;
	}
}

void CameraMovementCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (CrescentEngine::g_IsCameraFirstMove)
	{
		CrescentEngine::g_CameraLastXPosition = xPos;
		CrescentEngine::g_CameraLastYPosition = yPos;
		CrescentEngine::g_IsCameraFirstMove = false;
	}

	float xOffset = xPos - CrescentEngine::g_CameraLastXPosition;
	float yOffset = CrescentEngine::g_CameraLastYPosition - yPos; //Reversed since Y Coordinates go from bottom to top.
	CrescentEngine::g_CameraLastXPosition = xPos;

	CrescentEngine::g_CameraLastYPosition = yPos;

	if (CrescentEngine::g_CameraMode)
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
	//g_RenderingComponents.m_Framebuffer.ResizeFramebuffer(windowWidth, windowHeight);
	g_CoreSystems.m_Window.ResizeWindow((float)windowWidth, (float)windowHeight);
}

