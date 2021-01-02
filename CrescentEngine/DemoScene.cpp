#include "CrescentPCH.h"
#include "Core.h"
#include "Window.h"
#include "Editor.h"
#include "Models/DefaultShapes.h"
#include "Rendering/Material.h"
#include "Rendering/Shader.h"
#include "Rendering/Renderer.h"
#include "Memory/Resources.h"
#include "Utilities/Camera.h"
#include "Editor.h"
#include "Utilities/Timestep.h"
#include "Scene/Scene.h"
#include "Scene/Skybox.h"
#include "Rendering/PBRCapture.h"

CrescentEngine::Window m_Window;
CrescentEngine::Renderer* m_Renderer;
glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
CrescentEngine::Camera m_Camera = { glm::vec3(0.0f, 0.0f, 3.0f) }; //Setups our Camera.
CrescentEngine::Editor m_Editor; //Setups our ImGui context.
CrescentEngine::Timestep m_Timestep; //Setups our Timestep.
float m_LastFrameTime = 0.0f;

void ProcessKeyboardEvents(GLFWwindow* window);
void FramebufferResizeCallback(GLFWwindow* window, int windowWidth, int windowHeight);
void CameraAllowEulerCallback(GLFWwindow* window, int button, int action, int mods);
void CameraMovementCallback(GLFWwindow* window, double xPos, double yPos);
void CameraZoomCallback(GLFWwindow* window, double xOffset, double yOffset);

int mainYetToCome(int argc, int argv[])
{
	// Windowing ================================================================================

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_RESIZABLE, true);
	glfwWindowHint(GLFW_SAMPLES, 16);

	//Initializes GLFW.
	m_Window.CreateNewWindow("Crescent Engine", 1280.0f, 720.0f);

	//Callbacks
	m_Window.SetFramebufferCallback(FramebufferResizeCallback);
	m_Window.SetMouseCursorCallback(CameraMovementCallback);
	m_Window.SetMouseButtonCallback(CameraAllowEulerCallback);
	m_Window.SetMouseScrollCallback(CameraZoomCallback);

	// Rendering ================================================================================
	//Initialize Renderer

	//Basic Shapes
	CrescentEngine::Plane plane(16, 16);
	CrescentEngine::Sphere sphere(64, 64);
	CrescentEngine::Sphere tSphere(256, 256);
	CrescentEngine::Torus torus(2.0f, 0.4f, 32, 32);
	CrescentEngine::Cube cube;

	//Material Setup
	CrescentEngine::Material* materialPBR = m_Renderer->CreateMaterial();
	CrescentEngine::Shader* plasmaOrbShader = CrescentEngine::Resources::LoadShader("Plasma Orb", "Resources/Shaders/Custom/PlasmaOrb.vs", "Resources/Shaders/Custom/PlasmaOrb.fs");
	CrescentEngine::Material* materialPlasmaOrb = m_Renderer->CreateCustomMaterial(plasmaOrbShader);
	materialPlasmaOrb->m_FaceCullingEnabled = false;
	materialPlasmaOrb->m_BlendingEnabled = true;
	materialPlasmaOrb->m_BlendSource = GL_ONE;
	materialPlasmaOrb->m_BlendDestination = GL_ONE;
	materialPlasmaOrb->SetShaderTexture("TexPerlin", CrescentEngine::Resources::LoadTexture("Perlin Noise", "Resources/Textures/Perlin.png"), 0);
	materialPlasmaOrb->SetShaderFloat("Strength", 1.5f);
	materialPlasmaOrb->SetShaderFloat("Speed", 0.083f);

	//Camera
	m_ProjectionMatrix = glm::perspective(glm::radians(m_Camera.m_MouseZoom), ((float)m_Editor.RetrieveViewportWidth() / (float)m_Editor.RetrieveViewportHeight()), 0.2f, 100.0f);

	//Scene Setup
	CrescentEngine::SceneNode* mainTorus = CrescentEngine::Scene::CreateSceneNode(&torus, materialPBR);
	CrescentEngine::SceneNode* secondTorus = CrescentEngine::Scene::CreateSceneNode(&torus, materialPBR);
	CrescentEngine::SceneNode* thirdTorus = CrescentEngine::Scene::CreateSceneNode(&torus, materialPBR);
	CrescentEngine::SceneNode* plasmaOrb = CrescentEngine::Scene::CreateSceneNode(&torus, materialPBR);

	mainTorus->AddChildNode(secondTorus);
	secondTorus->AddChildNode(thirdTorus);

	mainTorus->SetScale(1.0f);
	mainTorus->SetPosition(glm::vec3(-4.4f, 3.46f, -0.3f));
	secondTorus->SetScale(0.65f);
	secondTorus->SetRotation(glm::vec4(0.0f, 1.0f, 0.0f, glm::radians(90.0f)));
	thirdTorus->SetScale(0.65f);

	plasmaOrb->SetPosition(glm::vec3(-4.0f, 4.0f, 0.25f));
	plasmaOrb->SetScale(0.6f);

	//Background
	CrescentEngine::Skybox* skybox = new CrescentEngine::Skybox;
	CrescentEngine::PBRCapture* pbrEnvironment = m_Renderer->GetSkyCapture();
	skybox->SetCubeMap(pbrEnvironment->m_Prefiltered);
	float lodLevel = 1.5f;
	skybox->m_Material->SetShaderFloat("lodLevel", lodLevel);

	//Post-Processing
	CrescentEngine::Shader* postProcessingShader1 = CrescentEngine::Resources::LoadShader("PostProcessing1", "Resources/Shaders/ScreenQuad.vs", "Resources/Shaders/CustomPostProcessing1.fs");
	CrescentEngine::Shader* postProcessingShader2 = CrescentEngine::Resources::LoadShader("PostProcessing2", "Resources/Shaders/ScreenQuad.vs", "Resources/Shaders/CustomPostProcessing2.fs");
	CrescentEngine::Material* customPostProcessingMaterial1 = m_Renderer->CreatePostProcessingMaterial(postProcessingShader1);
	CrescentEngine::Material* customPostProcessingMaterial2 = m_Renderer->CreatePostProcessingMaterial(postProcessingShader2);

	//Mesh
	CrescentEngine::SceneNode* sponzaModel = CrescentEngine::Resources::LoadMesh(m_Renderer, "Sponza", "Resources/Models/sponza/sponza.obj");
	sponzaModel->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f));
	sponzaModel->SetScale(0.01f);

	while (!m_Window.RetrieveWindowCloseStatus())
	{
		//Retrieve Delta Time
		float currentFrame = m_Window.RetrieveCurrentTime();
		m_Timestep = currentFrame - m_LastFrameTime;
		m_LastFrameTime = currentFrame;

		//Poll Events
		m_Window.PollEvents();
		ProcessKeyboardEvents(m_Window.RetrieveWindow());
		m_Renderer->ClearBuffers();

		mainTorus->SetRotation(glm::vec4(glm::vec3(1.0f, 0.0f, 0.0f), glfwGetTime() * 2.0));
		secondTorus->SetRotation(glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), glfwGetTime() * 3.0));
		thirdTorus->SetRotation(glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), glfwGetTime() * 4.0));

		m_Window.SwapBuffers();
	}

	m_Window.TerminateWindow();
	return 0;
}