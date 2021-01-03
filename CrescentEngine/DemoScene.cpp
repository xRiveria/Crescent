#include "CrescentPCH.h"
#include "Core.h"
#include "Window.h"
#include "Editor.h"
#include "Crescent.h"
#include "Utilities/Timestep.h"
#include "Utilities/Random.h"

Crescent::Window m_Window;
Crescent::Renderer* m_Renderer;
glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
Crescent::Camera m_Camera = { glm::vec3(0.0f, 0.0f, 3.0f) }; //Setups our Camera.
Crescent::Editor m_Editor; //Setups our ImGui context.
Crescent::Timestep m_Timestep; //Setups our Timestep.
float m_LastFrameTime = 0.0f;

void ProcessKeyboardEvents(GLFWwindow* window);
void FramebufferResizeCallback(GLFWwindow* window, int windowWidth, int windowHeight);
void CameraAllowEulerCallback(GLFWwindow* window, int button, int action, int mods);
void CameraMovementCallback(GLFWwindow* window, double xPos, double yPos);
void CameraZoomCallback(GLFWwindow* window, double xOffset, double yOffset);

int main(int argc, int argv[])
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

	int width, height;
	glfwGetFramebufferSize(m_Window.RetrieveWindow(), &width, &height);

	// Rendering ================================================================================
	//Initialize Renderer
	m_Renderer = Crescent::Initialize(m_Window.RetrieveWindow());
	m_Renderer->SetRenderViewportSize(width, height);
	m_Renderer->SetCamera(&m_Camera);

	//Basic Shapes
	Crescent::Plane plane(16, 16);
	Crescent::Sphere sphere(64, 64);
	Crescent::Sphere tSphere(256, 256);
	Crescent::Torus torus(2.0f, 0.4f, 32, 32);
	Crescent::Cube cube;

	//Material Setup
	Crescent::Material* materialPBR = m_Renderer->CreateMaterial();
	Crescent::Shader* plasmaOrbShader = Crescent::Resources::LoadShader("Plasma Orb", "Resources/Shaders/Custom/PlasmaOrb.vs", "Resources/Shaders/Custom/PlasmaOrb.fs");
	Crescent::Material* materialPlasmaOrb = m_Renderer->CreateCustomMaterial(plasmaOrbShader);
	materialPlasmaOrb->m_FaceCullingEnabled = false;
	materialPlasmaOrb->m_BlendingEnabled = true;
	materialPlasmaOrb->m_BlendSource = GL_ONE;
	materialPlasmaOrb->m_BlendDestination = GL_ONE;
	materialPlasmaOrb->SetShaderTexture("TexPerlin", Crescent::Resources::LoadTexture("Perlin Noise", "Resources/Textures/Perlin.png"), 0);
	materialPlasmaOrb->SetShaderFloat("Strength", 1.5f);
	materialPlasmaOrb->SetShaderFloat("Speed", 0.083f);

	//Camera
	m_Camera.SetPerspective(glm::radians(m_Camera.m_MouseZoom), m_Renderer->RetrieveRenderViewportSize().x / m_Renderer->RetrieveRenderViewportSize().y, 0.1f, 100.0f);

	//Scene Setup
	Crescent::SceneNode* mainTorus = Crescent::Scene::CreateSceneNode(&torus, materialPBR);
	Crescent::SceneNode* secondTorus = Crescent::Scene::CreateSceneNode(&torus, materialPBR);
	Crescent::SceneNode* thirdTorus = Crescent::Scene::CreateSceneNode(&torus, materialPBR);

	Crescent::SceneNode* plasmaOrb = Crescent::Scene::CreateSceneNode(&torus, materialPlasmaOrb);

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
	Crescent::Skybox* skybox = new Crescent::Skybox;
	Crescent::PBRCapture* pbrEnvironment = m_Renderer->GetSkyCapture();
	skybox->SetCubeMap(pbrEnvironment->m_Prefiltered);
	float lodLevel = 1.5f;
	skybox->m_Material->SetShaderFloat("lodLevel", lodLevel);

	//Post-Processing
	Crescent::Shader* postProcessingShader1 = Crescent::Resources::LoadShader("PostProcessing1", "Resources/Shaders/ScreenQuad.vs", "Resources/Shaders/CustomPostProcessing1.fs");
	Crescent::Shader* postProcessingShader2 = Crescent::Resources::LoadShader("PostProcessing2", "Resources/Shaders/ScreenQuad.vs", "Resources/Shaders/CustomPostProcessing2.fs");
	Crescent::Material* customPostProcessingMaterial1 = m_Renderer->CreatePostProcessingMaterial(postProcessingShader1);
	Crescent::Material* customPostProcessingMaterial2 = m_Renderer->CreatePostProcessingMaterial(postProcessingShader2);

	//Mesh
	Crescent::SceneNode* sponzaModel = Crescent::Resources::LoadMesh(m_Renderer, "Sponza", "Resources/Models/Sponza/sponza.obj");
	sponzaModel->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f));
	sponzaModel->SetScale(0.01f);

	//Lighting
	Crescent::DirectionalLight directionalLight;
	directionalLight.m_Direction = glm::vec3(0.2f, -1.0f, 0.25f);
	directionalLight.m_Color = glm::vec3(1.0f, 0.89f, 0.7f);
	directionalLight.m_Intensity = 50.0f;
	m_Renderer->AddLight(&directionalLight);

	std::vector<Crescent::PointLight> torchLights;
	{
		Crescent::PointLight torch;
		torch.m_Radius = 2.5f;
		torch.m_Color = glm::vec3(1.0f, 0.3f, 0.05f);
		torch.m_Intensity = 50.0f;
		torch.m_MeshRenderEnabled = true;

		torch.m_Position = glm::vec3(4.85f, 0.7f, 1.43f);
		torchLights.push_back(torch);
		torch.m_Position = glm::vec3(4.85f, 0.7f, -2.2f);
		torchLights.push_back(torch);
		torch.m_Position = glm::vec3(-6.19f, 0.7f, 1.43f);
		torchLights.push_back(torch);
		torch.m_Position = glm::vec3(-6.19f, 0.7f, -2.2f);
		torchLights.push_back(torch);

		m_Renderer->AddLight(&torchLights[0]);
		m_Renderer->AddLight(&torchLights[1]);
		m_Renderer->AddLight(&torchLights[2]);
		m_Renderer->AddLight(&torchLights[3]);
	}

	std::vector<Crescent::PointLight> randomLights;
	std::vector<glm::vec3> randomLightStartPositions;
	{
		for (int i = 0; i < 100; ++i)
		{
			Crescent::PointLight light;
			light.m_Radius = 1.0 + Random::Uniliteral() * 3.0f;
			light.m_Intensity = 10.0 + Random::Uniliteral() * 1000.0;
			light.m_Color = glm::vec3(Random::Uniliteral(), Random::Uniliteral(), Random::Uniliteral());
			light.m_MeshRenderEnabled = true;
			randomLights.push_back(light);
			randomLightStartPositions.push_back(glm::vec3(Random::Biliteral() * 12.0f, Random::Uniliteral() * 5.0f, Random::Biliteral() * 6.0f));
		}
	}

	{
		//Bake irradiance GI (with grid placement of probes).
		// bottom floor - center
		m_Renderer->AddIrradianceProbe(glm::vec3(0.0f, 0.5f, -0.5f), 3.25);
		m_Renderer->AddIrradianceProbe(glm::vec3(3.0f, 0.5f, -0.5f), 3.25);
		m_Renderer->AddIrradianceProbe(glm::vec3(6.0f, 0.5f, -0.5f), 3.25);
		m_Renderer->AddIrradianceProbe(glm::vec3(8.5f, 0.5f, -0.5f), 3.25);
		m_Renderer->AddIrradianceProbe(glm::vec3(11.4f, 0.5f, -0.5f), 4.25);
		m_Renderer->AddIrradianceProbe(glm::vec3(-3.0f, 0.5f, -0.5f), 3.25);
		m_Renderer->AddIrradianceProbe(glm::vec3(-6.2f, 0.5f, -0.5f), 3.25);
		m_Renderer->AddIrradianceProbe(glm::vec3(-9.5f, 0.5f, -0.5f), 3.25);
		m_Renderer->AddIrradianceProbe(glm::vec3(-12.1f, 0.5f, -0.5f), 4.25);
		// bottom floor - left wing	   
		m_Renderer->AddIrradianceProbe(glm::vec3(0.0f, 0.5f, 4.0f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(4.0f, 0.5f, 4.0f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(8.0f, 0.5f, 4.0f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(12.0f, 0.5f, 4.0f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-4.0f, 0.5f, 4.0f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-8.0f, 0.5f, 4.0f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-12.0f, 0.5f, 4.0f), 4.0);
		// bottom floor - right wing   
		m_Renderer->AddIrradianceProbe(glm::vec3(0.0f, 0.5f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(4.0f, 0.5f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(8.0f, 0.5f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(12.0f, 0.5f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-4.0f, 0.5f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-8.0f, 0.5f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-12.0f, 0.5f, -4.5f), 4.0);
		// 1st floor - center wing	   
		m_Renderer->AddIrradianceProbe(glm::vec3(0.0f, 5.0f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(4.0f, 5.0f, -0.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(8.0f, 5.0f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(12.0f, 5.0f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(-4.0f, 5.0f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(-8.0f, 5.0f, -0.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-12.0f, 5.0f, -0.5f), 4.5);
		// 1st floor - left wing	  
		m_Renderer->AddIrradianceProbe(glm::vec3(0.0f, 5.0f, 4.0), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(4.0f, 5.0f, 4.0), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(8.0f, 5.0f, 4.0), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(12.0f, 5.0f, 4.0), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-4.0f, 5.0f, 4.0), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-8.0f, 5.0f, 4.0), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-11.5f, 5.0f, 4.0), 4.0);
		// 1st floor - right wing	  
		m_Renderer->AddIrradianceProbe(glm::vec3(0.0f, 5.0f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(4.0f, 5.0f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(8.0f, 5.0f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(12.0f, 5.0f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-4.0f, 5.0f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-8.0f, 5.0f, -4.5f), 4.0);
		m_Renderer->AddIrradianceProbe(glm::vec3(-11.5f, 5.0f, -4.5f), 4.0);
		// 2nd floor - center wing	  
		m_Renderer->AddIrradianceProbe(glm::vec3(0.0f, 9.5f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(4.0f, 9.5f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(8.0f, 9.5f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(12.0f, 9.5f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(-4.0f, 9.5f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(-8.0f, 9.5f, -0.5f), 4.5);
		m_Renderer->AddIrradianceProbe(glm::vec3(-11.5f, 9.5f, -0.5f), 4.5);

		// bake before rendering
		m_Renderer->BakeProbes();
	}


	while (!m_Window.RetrieveWindowCloseStatus())
	{
		//Retrieve Delta Time
		float currentFrame = m_Window.RetrieveCurrentTime();
		m_Timestep = currentFrame - m_LastFrameTime;
		m_LastFrameTime = currentFrame;

		//Poll Events
		m_Window.PollEvents();
		ProcessKeyboardEvents(m_Window.RetrieveWindow());

		//Update Camera Render Logic
		m_Camera.UpdateCameraVectors();

		mainTorus->SetRotation(glm::vec4(glm::vec3(1.0f, 0.0f, 0.0f), glfwGetTime() * 2.0));
		secondTorus->SetRotation(glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), glfwGetTime() * 3.0));
		thirdTorus->SetRotation(glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), glfwGetTime() * 4.0));

		for (int i = 0; i < torchLights.size(); ++i)
		{
			torchLights[i].m_Radius = 1.5f + 0.1 * std::cos(std::sin(glfwGetTime() * 1.37 + i * 7.31) * 3.1 + i);
			torchLights[i].m_Intensity = 25.0f + 5.0 * std::cos(std::sin(glfwGetTime() * 0.67 + i * 2.31) * 2.31 * i);
		}

		for (int i = 0; i < randomLights.size(); ++i)
		{
			randomLights[i].m_Position = randomLightStartPositions[i] + glm::vec3(std::sin(glfwGetTime() * 0.31f + i * 3.17f) * 1.79f, std::cos(glfwGetTime() * 0.21f + i * 1.11f) * 1.61f, std::sin(glfwGetTime() * 0.49 + i * 0.79f) * 1.31);
		}

		materialPlasmaOrb->SetShaderFloat("Time", glfwGetTime());

		m_Renderer->PushRenderCommand(mainTorus);
		m_Renderer->PushRenderCommand(sponzaModel);
		m_Renderer->PushRenderCommand(plasmaOrb);
		m_Renderer->PushRenderCommand(skybox);

		//Core render function.
		m_Renderer->RenderCommandQueueObjects();

		m_Window.SwapBuffers();
	}

	m_Window.TerminateWindow();
	return 0;
}