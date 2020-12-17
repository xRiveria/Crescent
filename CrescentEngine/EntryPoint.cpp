#include "CrescentPCH.h"
#include "OpenGL/OpenGLRenderer.h"
#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include "OpenGL/OpenGLRenderer.h"
#include "OpenGL/IndexBuffer.h"
#include "OpenGL/VertexBuffer.h"
#include "OpenGL/VertexArray.h"
#include "OpenGL/Shader.h"
#include "OpenGL/VertexBufferLayout.h"
#include "OpenGL/Texture.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "LearnShader.h"
#include "stb_image/stb_image.h"
#include <glm/gtc/type_ptr.hpp>
#include "Utilities/Camera.h"
#include "Editor.h"
#include "imgui/imgui.h"
#include "Models/Model.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Window.h"
#include "Utilities/Cubemap.h"

//To Do:
//Cleanup Entry Point.
//Abstract Texture Class.
float animationTime = 0.0f;
bool cameraMode = false;

std::vector<std::string> cubemapFaces{ //In order of the cubemap enums.
    "Resources/Skybox/Ocean/right.jpg",
    "Resources/Skybox/Ocean/left.jpg",
    "Resources/Skybox/Ocean/top.jpg",
    "Resources/Skybox/Ocean/bottom.jpg",
    "Resources/Skybox/Ocean/front.jpg",
    "Resources/Skybox/Ocean/back.jpg"
};

//Directional Light
glm::vec3 directionalLight_LightDirection = { -0.2f, -1.0f, -0.3f };
glm::vec3 directionalLight_AmbientIntensity = { 0.2f, 0.2f, 0.2f };
glm::vec3 directionalLight_DiffuseIntensity = { 0.5f, 0.5f, 0.5f };
glm::vec3 directionalLight_SpecularIntensity = { 1.0f, 1.0f, 1.0f };

//Point Light 1
glm::vec3 pointLight1_AmbientIntensity = { 0.2f, 0.2f, 0.2f };
glm::vec3 pointLight1_DiffuseIntensity = { 0.5f, 0.5f, 0.5f };
glm::vec3 pointLight1_SpecularIntensity = { 1.0f, 1.0f, 1.0f };

//Point Light 2
glm::vec3 pointLight2_AmbientIntensity = { 0.2f, 0.2f, 0.2f };
glm::vec3 pointLight2_DiffuseIntensity = { 0.5f, 0.5f, 0.5f };
glm::vec3 pointLight2_SpecularIntensity = { 1.0f, 1.0f, 1.0f };

//Point Light 3
glm::vec3 pointLight3_AmbientIntensity = { 0.2f, 0.2f, 0.2f };
glm::vec3 pointLight3_DiffuseIntensity = { 0.5f, 0.5f, 0.5f };
glm::vec3 pointLight3_SpecularIntensity = { 1.0f, 1.0f, 1.0f };

//Point Light 4
glm::vec3 pointLight4_AmbientIntensity = { 0.2f, 0.2f, 0.2f };
glm::vec3 pointLight4_DiffuseIntensity = { 0.5f, 0.5f, 0.5f };
glm::vec3 pointLight4_SpecularIntensity = { 1.0f, 1.0f, 1.0f };

glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
};

//Spotlight
float spotLight_InnerLightCutoff = 12.5f;
float spotLight_OuterLightCutoff = 17.5f;
glm::vec3 spotLight_AmbientIntensity = { 0.2f, 0.2f, 0.2f };
glm::vec3 spotLight_DiffuseIntensity = { 0.5f, 0.5f, 0.5f };
glm::vec3 spotLight_SpecularIntensity = { 1.0f, 1.0f, 1.0f };

float deltaTime = 0.0f;	// Time between current frame and last frame.
float lastFrame = 0.0f; // Time of last frame.

//Camera
Camera g_Camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool firstMove = true;
float lastX = 400;
float lastY = 300;

//Light
glm::vec3 g_AmbientIntensity = { 0.2f, 0.2f, 0.2f };
glm::vec3 g_AmbientColor = { 1.0f, 0.5f, 0.31f };

glm::vec3 g_DiffuseIntensity = { 0.5f, 0.5f, 0.5f }; 
glm::vec3 g_DiffuseColor = { 1.0f, 0.5f, 0.31f };

glm::vec3 g_SpecularIntensity = { 1.0f, 1.0f, 1.0f };
float g_SpecularScattering = 32.0f;
glm::vec3 g_SpecularColor = { 1.0f, 1.0f, 1.0f };

//Settings
unsigned int m_ScreenWidth = 800;
unsigned int m_ScreenHeight = 600;
float visibleValue = 0.1f;

//Lighting
glm::vec3 lightPosition = { 0.0f, 1.0f, 2.0f };
glm::vec3 modelPosition = { -6.0f, -3.0f, 5.0f };
glm::vec3 guardRotation = { -5.0f, 0.0f, 0.0f };
glm::vec3 guardPosition = { -40.0f, -3.0f, 36.0f };
glm::vec3 reflectModelPosition = { 3.0f, 0.0f, 0.0f };
glm::vec3 planePosition = { 0.0f, 0.0f, 0.0f };
bool refractOn = true;
float guardRotationAngle = 0.0f;

CrescentEngine::Editor m_Editor;

//This is a callback function that is called whenever a window is resized.
void FramebufferResizeCallback(GLFWwindow* window, int windowWidth, int windowHeight)
{
    glViewport(0, 0, windowWidth, windowHeight);
    m_ScreenHeight = windowHeight;
    m_ScreenWidth = windowWidth; 
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void ProcessInput(GLFWwindow* window);
void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

unsigned int LoadTexture(const std::string& filePath);

glm::vec3 cubePositions[] = {
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

CrescentEngine::Window m_Window;
CrescentEngine::Cubemap m_Cubemap;

int mainDefunct()
{
    //CrescentEngine::Renderer::InitializeSelectedRenderer(CrescentEngine::Renderer::API::OpenGL);
    m_Window.CreateWindow("Crescent Engine", 1920.0f, 1080.0f);
    
    //Sets a callback to a viewport resize function everytime we resize our window
    glfwSetFramebufferSizeCallback(m_Window.RetrieveWindow(), FramebufferResizeCallback);
    glfwSetMouseButtonCallback(m_Window.RetrieveWindow(), MouseButtonCallback);
    glfwSetCursorPosCallback(m_Window.RetrieveWindow(), MouseCallback);
    glfwSetScrollCallback(m_Window.RetrieveWindow(), ScrollCallback);

    m_Editor.SetApplicationContext(m_Window.RetrieveWindow());
    m_Editor.InitializeImGui();

    //m_Window.InitializeOpenGL();

    glEnable(GL_DEPTH_TEST);

    //===

    LearnShader lightingShader("Resources/Shaders/VertexShader.shader", "Resources/Shaders/FragmentShader.shader");
    LearnShader lightCubeShader("Resources/Shaders/LightVertexShader.shader", "Resources/Shaders/LightFragmentShader.shader");
    LearnShader animationShader("Resources/Shaders/AnimationVertex.shader", "Resources/Shaders/AnimationFragment.shader");
    LearnShader reflectiveShader("Resources/Shaders/ReflectiveVertex.shader", "Resources/Shaders/ReflectiveFragment.shader");

    m_Cubemap.LoadCubemap(cubemapFaces);

    //Because OpenGL works in 3D space, we render a 2D triangle with each vertex having a Z coordinate of 0.0. This way, the depth of the triangle remains the same, making it look like its 2D. 
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    unsigned int indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };
    
    unsigned int vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject); //Every subsequent attribute pointer call will now link the buffer and said attribute configurations to this vertex array object.

    unsigned int vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int indexBufferObject;
	glGenBuffers(1, &indexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //Links the vertex attributes from the buffers that we passed into the shaders.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //Light Source VBO

    unsigned int lightVertexArrayObject;
    glGenVertexArrays(1, &lightVertexArrayObject);
    glBindVertexArray(lightVertexArrayObject);
    //We only need to bind to the VBO, the container's VBO data also ready contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    //Set the vertex attribute.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Cubemap
    m_Cubemap.SetupCubemap();

   // Texture
    unsigned int diffuseMap = LoadTexture("Resources/Textures/Container2.png");
    unsigned int specularMap = LoadTexture("Resources/Textures/ContainerSpecularMap.png");
    unsigned int emissionMap = LoadTexture("Resources/Textures/ContainerEmissionTexture.jpg");
    //unsigned int colorUniformLocation = glGetUniformLocation(shaderProgram, "ourColor");
    //glUseProgram(shaderProgram);
     lightingShader.UseShader(); //Always activate shaders before setting uniforms.
     lightingShader.SetUniformInteger("material.diffuseMap", 0);
     lightingShader.SetUniformInteger("material.specularMap", 1);
     lightingShader.SetUniformInteger("material.emissionMap", 2);
    // ourShader.SetUniformInteger("texture2", 1);

     //Model
     stbi_set_flip_vertically_on_load(true);
     LearnShader staticModelShader("Resources/Shaders/StaticModelVertex.shader", "Resources/Shaders/StaticModelFragment.shader");
     CrescentEngine::Model ourModel("Resources/Models/Backpack/backpack.obj");
     CrescentEngine::Model lamp("Resources/Models/RedstoneLamp/Redstone-lamp.obj");
     CrescentEngine::Model girl("Resources/Models/Girl/Character_Girl.fbx");

     stbi_set_flip_vertically_on_load(false);
     CrescentEngine::Model slime("Resources/Models/Torg/Torg_Animal.fbx");
     std::shared_ptr<CrescentEngine::Model> guard = std::make_shared<CrescentEngine::Model>("Resources/Models/GuardBob/boblampclean.md5mesh");
     stbi_set_flip_vertically_on_load(true);


     while (!glfwWindowShouldClose(m_Window.RetrieveWindow()))
     {
         float currentFrame = glfwGetTime();

         deltaTime = currentFrame - lastFrame;
         lastFrame = currentFrame;
         animationTime += deltaTime;

         glfwPollEvents();
         ProcessInput(m_Window.RetrieveWindow()); //Process key input events.

         /// ===== Rendering =====
         //We want to place all the rendering commands in the render loop, since we want to execute all the rendering commands each iteration or frame of the loop. 
         //Lets clear the screen with a color of our choice. At the start of each frame, we want to clear the screen.
         //Otherwise, we would still see results from the previous frame, which could of course be the effect you're looking for, but usually, we don't.
         //We can clear the screen's color buffer using "glClear()", where we pass in buffer bits to specify which buffer we would like to clear.
         //The possible bits we can set are GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT and GL_STENCIL_BUFFER_BIT. Right now, we only care about the color values, so we only clear the color buffer.

         //Note that we also specify the color to clear the screen with using "glClearColor()".
         //Whenever we call "glClear()" and clear the color buffer, the entire color buffer will be filled with the color as configured with "glClearColor()".
         //As you may recall, the "glClearColor()" function is a state setting function and "glClear()" is a state using function in that it uses the current state to retrieve the clear color from.
         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         //Be sure to activate shader when setting uniforms/drawing objects.
         reflectiveShader.UseShader();
         reflectiveShader.SetUniformVector3("cameraPos", g_Camera.m_CameraPosition);
         reflectiveShader.SetUniformBool("refractOn", refractOn);
         lightingShader.UseShader();
         //lightingShader.SetUniformVector3("light.lightPosition", g_Camera.m_CameraPosition);
         //lightingShader.SetUniformVector3("light.lightDirection", g_Camera.m_CameraFront); //Note that we define the direction as a direction from the light source; you can quickly see that the light's direction is pointing downwards.
         lightingShader.SetUniformVector3("viewPosition", g_Camera.m_CameraPosition);
         lightingShader.SetUniformInteger("skybox", 0);

         lightingShader.SetUniformVector3("pointLights[0].lightPosition", pointLightPositions[0]);
         lightingShader.SetUniformFloat("pointLights[0].attenuationConstant", 1.0f);
         lightingShader.SetUniformFloat("pointLights[0].attenuationLinear", 0.09f);
         lightingShader.SetUniformFloat("pointLights[0].attenuationQuadratic", 0.032f);
         lightingShader.SetUniformVector3("pointLights[0].ambientIntensity", pointLight1_AmbientIntensity);
         lightingShader.SetUniformVector3("pointLights[0].diffuseIntensity", pointLight1_DiffuseIntensity);
         lightingShader.SetUniformVector3("pointLights[0].specularIntensity", pointLight1_SpecularIntensity);

         lightingShader.SetUniformVector3("pointLights[1].lightPosition", pointLightPositions[1]);
         lightingShader.SetUniformFloat("pointLights[1].attenuationConstant", 1.0f);
         lightingShader.SetUniformFloat("pointLights[1].attenuationLinear", 0.09f);
         lightingShader.SetUniformFloat("pointLights[1].attenuationQuadratic", 0.032f);
         lightingShader.SetUniformVector3("pointLights[1].ambientIntensity", pointLight2_AmbientIntensity);
         lightingShader.SetUniformVector3("pointLights[1].diffuseIntensity", pointLight2_DiffuseIntensity);
         lightingShader.SetUniformVector3("pointLights[1].specularIntensity", pointLight2_SpecularIntensity);

         lightingShader.SetUniformVector3("pointLights[2].lightPosition", pointLightPositions[2]);
         lightingShader.SetUniformFloat("pointLights[2].attenuationConstant", 1.0f);
         lightingShader.SetUniformFloat("pointLights[2].attenuationLinear", 0.09f);
         lightingShader.SetUniformFloat("pointLights[2].attenuationQuadratic", 0.032f);
         lightingShader.SetUniformVector3("pointLights[2].ambientIntensity", pointLight3_AmbientIntensity);
         lightingShader.SetUniformVector3("pointLights[2].diffuseIntensity", pointLight3_DiffuseIntensity);
         lightingShader.SetUniformVector3("pointLights[2].specularIntensity", pointLight3_SpecularIntensity);

         lightingShader.SetUniformVector3("pointLights[3].lightPosition", pointLightPositions[3]);
         lightingShader.SetUniformFloat("pointLights[3].attenuationConstant", 1.0f);
         lightingShader.SetUniformFloat("pointLights[3].attenuationLinear", 0.09f);
         lightingShader.SetUniformFloat("pointLights[3].attenuationQuadratic", 0.032f);
         lightingShader.SetUniformVector3("pointLights[3].ambientIntensity", pointLight4_AmbientIntensity);
         lightingShader.SetUniformVector3("pointLights[3].diffuseIntensity", pointLight4_DiffuseIntensity);
         lightingShader.SetUniformVector3("pointLights[3].specularIntensity", pointLight4_SpecularIntensity);

         //Directional Light
         lightingShader.SetUniformVector3("directionalLight.lightDirection", directionalLight_LightDirection);
         lightingShader.SetUniformVector3("directionalLight.ambientIntensity", directionalLight_AmbientIntensity);
         lightingShader.SetUniformVector3("directionalLight.diffuseIntensity", directionalLight_DiffuseIntensity);
         lightingShader.SetUniformVector3("directionalLight.specularIntensity", directionalLight_SpecularIntensity);

         //Spotlight
         lightingShader.SetUniformVector3("spotLight.lightPosition", g_Camera.m_CameraPosition);
         lightingShader.SetUniformVector3("spotLight.lightDirection", g_Camera.m_CameraFront);
         lightingShader.SetUniformFloat("spotLight.innerLightCutoff", glm::cos(glm::radians(spotLight_InnerLightCutoff)));
         lightingShader.SetUniformFloat("spotLight.outerLightCutoff", glm::cos(glm::radians(spotLight_OuterLightCutoff)));

         lightingShader.SetUniformFloat("spotLight.attenuationConstant", 1.0f);
         lightingShader.SetUniformFloat("spotLight.attenuationLinear", 0.09f);
         lightingShader.SetUniformFloat("spotLight.attenuationQuadratic", 0.032f);

         lightingShader.SetUniformVector3("spotLight.ambientIntensity", spotLight_AmbientIntensity);
         lightingShader.SetUniformVector3("spotLight.diffuseIntensity", spotLight_DiffuseIntensity);
         lightingShader.SetUniformVector3("spotLight.specularIntensity", spotLight_SpecularIntensity);

         //Backpack
         staticModelShader.UseShader();
         staticModelShader.SetUniformVector3("viewPosition", g_Camera.m_CameraPosition);
         staticModelShader.SetUniformVector3("pointLight.lightPosition", pointLightPositions[0]);
         //backpackShader.SetUniformVector3("spotLight.lightDirection", g_Camera.m_CameraFront);
         //backpackShader.SetUniformFloat("spotLight.innerLightCutoff", glm::cos(glm::radians(spotLight_InnerLightCutoff)));
         //backpackShader.SetUniformFloat("spotLight.outerLightCutoff", glm::cos(glm::radians(spotLight_OuterLightCutoff)));

         staticModelShader.SetUniformFloat("pointLight.attenuationConstant", 1.0f);
         staticModelShader.SetUniformFloat("pointLight.attenuationLinear", 0.09f);
         staticModelShader.SetUniformFloat("pointLight.attenuationQuadratic", 0.032f);

         staticModelShader.SetUniformVector3("pointLight.ambientIntensity", spotLight_AmbientIntensity);
         staticModelShader.SetUniformVector3("pointLight.diffuseIntensity", spotLight_DiffuseIntensity);
         staticModelShader.SetUniformVector3("pointLight.specularIntensity", spotLight_SpecularIntensity);

         //Directional Light
         staticModelShader.SetUniformVector3("directionalLight.lightDirection", directionalLight_LightDirection);
         staticModelShader.SetUniformVector3("directionalLight.ambientIntensity", directionalLight_AmbientIntensity);
         staticModelShader.SetUniformVector3("directionalLight.diffuseIntensity", directionalLight_DiffuseIntensity);
         staticModelShader.SetUniformVector3("directionalLight.specularIntensity", directionalLight_SpecularIntensity);

         //Animation Shader
         animationShader.UseShader();
         animationShader.SetUniformVectorMat4("uBoneMatrices", guard->m_BoneMatrices);
         animationShader.SetUniformVector3("viewPosition", g_Camera.m_CameraPosition);
         animationShader.SetUniformVector3("pointLight.lightPosition", pointLightPositions[0]);
         animationShader.SetUniformFloat("pointLight.attenuationConstant", 1.0f);
         animationShader.SetUniformFloat("pointLight.attenuationLinear", 0.09f);
         animationShader.SetUniformFloat("pointLight.attenuationQuadratic", 0.032f);

         animationShader.SetUniformVector3("pointLight.ambientIntensity", spotLight_AmbientIntensity);
         animationShader.SetUniformVector3("pointLight.diffuseIntensity", spotLight_DiffuseIntensity);
         animationShader.SetUniformVector3("pointLight.specularIntensity", spotLight_SpecularIntensity);

         //Directional Light
         animationShader.SetUniformVector3("directionalLight.lightDirection", directionalLight_LightDirection);
         animationShader.SetUniformVector3("directionalLight.ambientIntensity", directionalLight_AmbientIntensity);
         animationShader.SetUniformVector3("directionalLight.diffuseIntensity", directionalLight_DiffuseIntensity);
         animationShader.SetUniformVector3("directionalLight.specularIntensity", directionalLight_SpecularIntensity);

         //lightingShader.SetUniformVector3("light.ambientIntensity", g_AmbientIntensity);
         //lightingShader.SetUniformVector3("material.ambientColor", g_AmbientColor);

         //lightingShader.SetUniformVector3("light.diffuseIntensity", g_DiffuseIntensity);
         //lightingShader.SetUniformVector3("material.diffuseColor", g_DiffuseColor);
         lightingShader.UseShader();
         lightingShader.SetUniformVector3("material.specularColor", g_SpecularColor);
         //lightingShader.SetUniformVector3("light.specularIntensity", g_SpecularIntensity);
         lightingShader.SetUniformFloat("material.specularScatter", g_SpecularScattering);
         //Our Object Cube

         //View/Projection Transformations
         glm::mat4 projectionMatrix = glm::perspective(glm::radians(g_Camera.m_MouseZoom), 800.0f / 600.0f, 0.1f, 100.0f);
         glm::mat4 viewMatrix = g_Camera.GetViewMatrix();
         lightingShader.SetUniformMat4("projection", projectionMatrix);
         lightingShader.SetUniformMat4("view", viewMatrix);

         glm::mat4 modelMatrix = glm::mat4(1.0f);
         lightingShader.SetUniformMat4("model", modelMatrix); //World Transformation

         //Render our Cube
         glm::mat4 cubeObjectMatrix = glm::mat4(1.0f);
         cubeObjectMatrix = glm::scale(modelMatrix, glm::vec3(1.5, 1.5, 1.5));
         cubeObjectMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0));
         // cubeObjectMatrix = glm::rotate(modelMatrix, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f)); //Rotation on the X-axis and Y axis.

         lightingShader.SetUniformMat4("model", cubeObjectMatrix);

         //Bind Diffuse Map
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, diffuseMap);

         glActiveTexture(GL_TEXTURE1);
         glBindTexture(GL_TEXTURE_2D, specularMap);

         glActiveTexture(GL_TEXTURE2);
         glBindTexture(GL_TEXTURE_2D, emissionMap);

         glBindVertexArray(vertexArrayObject);
         //glDrawArrays(GL_TRIANGLES, 0, 36);


         for (size_t i = 0; i < 9; i++)
         {
             glm::mat4 model = glm::mat4(1.0f);
             model = glm::translate(model, cubePositions[i] + glm::vec3(0.0f, 0.0f, -10.0f));
             float angle = 20.0f * i;
             model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
             lightingShader.SetUniformMat4("model", model);
             glDrawArrays(GL_TRIANGLES, 0, 36);
         }

         //Draw our Lamp Object

         for (int i = 0; i < 4; i++)
         {
             lightCubeShader.UseShader();
             lightCubeShader.SetUniformMat4("projection", projectionMatrix);
             lightCubeShader.SetUniformMat4("view", viewMatrix);

             modelMatrix = glm::mat4(1.0f);
             modelMatrix = glm::translate(modelMatrix, pointLightPositions[i]);
             modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f)); //A smaller cube.     
             lightCubeShader.SetUniformMat4("model", modelMatrix);

             //lightCubeShader.SetUniformVector3("lightColor", g_DiffuseColor);
             glBindVertexArray(lightVertexArrayObject);
             lamp.Draw(lightCubeShader);
         }

         //Reflective Cube
         reflectiveShader.UseShader();
         reflectiveShader.SetUniformMat4("projection", projectionMatrix);
         reflectiveShader.SetUniformMat4("view", viewMatrix);
         glm::mat4 reflectModel = glm::mat4(1.0f);
         reflectModel = glm::translate(reflectModel, reflectModelPosition);
         reflectiveShader.SetUniformMat4("model", reflectModel);
         glBindVertexArray(vertexArrayObject);
         glDrawArrays(GL_TRIANGLES, 0, 36);

         //Render the loaded model.
         staticModelShader.UseShader();
         staticModelShader.SetUniformMat4("projection", projectionMatrix);
         staticModelShader.SetUniformMat4("view", viewMatrix);
         glm::mat4 backpackModel = glm::mat4(1.0f);
         backpackModel = glm::translate(backpackModel, glm::vec3(0.0f, 0.0f, 0.0f));
         backpackModel = glm::scale(backpackModel, glm::vec3(1.0f, 1.0f, 1.0f));
         staticModelShader.SetUniformMat4("model", backpackModel);
         ourModel.Draw(staticModelShader);

         glm::mat4 torgModel = glm::mat4(1.0f);
         torgModel = glm::scale(torgModel, glm::vec3(0.5f)); //A smaller cube.     
         torgModel = glm::translate(torgModel, modelPosition);
         torgModel = glm::rotate(torgModel, (float)glfwGetTime(), glm::vec3(0.0f, 0.5f, 0.0f));
         staticModelShader.SetUniformMat4("model", torgModel);
         girl.Draw(staticModelShader);

         animationShader.UseShader();
         animationShader.SetUniformMat4("projection", projectionMatrix);
         animationShader.SetUniformMat4("view", viewMatrix);
         glm::mat4 guardModel = glm::mat4(1.0f);
         guardModel = glm::scale(guardModel, glm::vec3(0.02f)); //A smaller cube.     
         guardModel = glm::translate(guardModel, guardPosition);
         guardModel = glm::rotate(guardModel, glm::radians(guardRotationAngle), guardRotation);
         animationShader.SetUniformMat4("model", guardModel);
         guard->Draw(0, animationTime, true, animationShader);

        g_Camera.UpdateCameraVectors();

        m_Editor.BeginEditorRenderLoop();

        //Point Lights
        ImGui::Begin("Camera Settings");
        ImGui::Text("Camera");
        ImGui::DragFloat3("Slime Position", glm::value_ptr(modelPosition), 0.1f);
        ImGui::DragFloat3("Camera Position", glm::value_ptr(g_Camera.m_CameraPosition), 0.2f);
        ImGui::DragFloat3("Guard Position", glm::value_ptr(guardPosition), 0.1f);
        ImGui::DragFloat3("Reflect Model Position", glm::value_ptr(reflectModelPosition), 0.1f);
        ImGui::Checkbox("Refract On or Off", &refractOn);
        ImGui::DragFloat3("Plane Position", glm::value_ptr(planePosition), 0.1f);
        ImGui::SameLine();
        if (refractOn == true)
        {
            ImGui::Text("Current State: Refraction");
        }
        else
        {
            ImGui::Text("Current State: Reflection");
        }
        ImGui::DragFloat("Guard Rotation Angle", &guardRotationAngle);
        ImGui::DragFloat("Camera FOV", &g_Camera.m_MouseZoom, 0.2f);
        ImGui::DragFloat("Camera Yaw", &g_Camera.m_CameraYaw, 0.2f);
        ImGui::DragFloat("Camera Pitch", &g_Camera.m_CameraPitch, 0.2f);
        ImGui::End();

        ImGui::Begin("Lights");
        ImGui::DragFloat("Specular Highlight", &g_SpecularScattering, 0.2f, 2.0f, 256.0f);
        ImGui::Begin("Directional Light");
        ImGui::DragFloat3("Light Direction", glm::value_ptr(directionalLight_LightDirection), 0.1f);
        ImGui::DragFloat3("Ambient Intensity##Direction", glm::value_ptr(directionalLight_AmbientIntensity), 0.1f);
        ImGui::DragFloat3("Diffuse Intensity##Direction", glm::value_ptr(directionalLight_DiffuseIntensity), 0.1f);
        ImGui::DragFloat3("Specular Intensity##Direction", glm::value_ptr(directionalLight_SpecularIntensity), 0.1f);
        ImGui::End();

        ImGui::Begin("Point Light 1");
        ImGui::DragFloat3("Light Position##1", glm::value_ptr(pointLightPositions[0]), 0.1f);
        ImGui::DragFloat3("Ambient Intensity##1", glm::value_ptr(pointLight1_AmbientIntensity), 0.1f);
        ImGui::DragFloat3("Diffuse Intensity##1", glm::value_ptr(pointLight1_DiffuseIntensity), 0.1f);
        ImGui::DragFloat3("Specular Intensity##1", glm::value_ptr(pointLight1_SpecularIntensity), 0.1f);
        ImGui::End();

        ImGui::Begin("Point Light 2");
        ImGui::DragFloat3("Light Position##2", glm::value_ptr(pointLightPositions[1]), 0.1f);
        ImGui::DragFloat3("Ambient Intensity##2", glm::value_ptr(pointLight2_AmbientIntensity), 0.1f);
        ImGui::DragFloat3("Diffuse Intensity##2", glm::value_ptr(pointLight2_DiffuseIntensity), 0.1f);
        ImGui::DragFloat3("Specular Intensity##2", glm::value_ptr(pointLight2_SpecularIntensity), 0.1f);
        ImGui::End();

        ImGui::Begin("Point Light 3");
        ImGui::DragFloat3("Light Position##3", glm::value_ptr(pointLightPositions[2]), 0.1f);
        ImGui::DragFloat3("Ambient Intensity##3", glm::value_ptr(pointLight3_AmbientIntensity), 0.1f);
        ImGui::DragFloat3("Diffuse Intensity##3", glm::value_ptr(pointLight3_DiffuseIntensity), 0.1f);
        ImGui::DragFloat3("Specular Intensity##3", glm::value_ptr(pointLight3_SpecularIntensity), 0.1f);
        ImGui::End();

        ImGui::Begin("Point Light 4");
        ImGui::DragFloat3("Light Position##4", glm::value_ptr(pointLightPositions[3]), 0.1f);
        ImGui::DragFloat3("Ambient Intensity##4", glm::value_ptr(pointLight4_AmbientIntensity), 0.1f);
        ImGui::DragFloat3("Diffuse Intensity##4", glm::value_ptr(pointLight4_DiffuseIntensity), 0.1f);
        ImGui::DragFloat3("Specular Intensity##4", glm::value_ptr(pointLight4_SpecularIntensity), 0.1f);
        ImGui::End();

        ImGui::Begin("Spotlight");
        ImGui::DragFloat("Inner Cutoff", &spotLight_InnerLightCutoff, 0.1f);
        ImGui::DragFloat("Outer Cutoff", &spotLight_OuterLightCutoff, 0.1f);
        ImGui::DragFloat3("Ambient Intensity##Spotlight", glm::value_ptr(spotLight_AmbientIntensity), 0.1f);
        ImGui::DragFloat3("Diffuse Intensity##Spotlight", glm::value_ptr(spotLight_DiffuseIntensity), 0.1f);
        ImGui::DragFloat3("Specular Intensity##Spotlight", glm::value_ptr(spotLight_SpecularIntensity), 0.1f);
        ImGui::End();

        //ImGui::NewLine();
        //ImGui::ColorEdit3("Ambient Color", glm::value_ptr(g_AmbientColor));
        //ImGui::NewLine();
        //ImGui::ColorEdit3("Diffuse Color", glm::value_ptr(g_DiffuseColor));
        //ImGui::NewLine();
        //ImGui::ColorEdit3("Specular Color", glm::value_ptr(g_SpecularColor));
        ImGui::End();

        ImGui::Begin("Frames");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0 / double(ImGui::GetIO().Framerate), double(ImGui::GetIO().Framerate));
        ImGui::End();

        m_Editor.EndEditorRenderLoop();

        //Draw Skybox as Last.
        m_Cubemap.DrawCubemap(viewMatrix, projectionMatrix);

        glfwSwapBuffers(m_Window.RetrieveWindow());
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vertexArrayObject);
    glDeleteVertexArrays(1, &lightVertexArrayObject);
    glDeleteBuffers(1, &vertexBufferObject);

    glfwTerminate();
    return 0;
}

/// ===== Input =====

void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);     
    }
   
    const float cameraSpeed = deltaTime * 2.5f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        g_Camera.ProcessKeyboardEvents(CameraMovement::Forward, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_Camera.ProcessKeyboardEvents(CameraMovement::Backward, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        g_Camera.ProcessKeyboardEvents(CameraMovement::Left, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_Camera.ProcessKeyboardEvents(CameraMovement::Right, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        animationTime = 0.0f;
    }
}

void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    g_Camera.ProcessMouseScroll(yOffset);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        cameraMode = true;
    }
    else
    {
        cameraMode = false;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMove)
    {
        lastX = xPos;
        lastY = yPos;
        firstMove = false;
    }

    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos; //Reversed since Y Coordinates go from bottom to top.
    lastX = xPos;

    lastY = yPos;

    if (cameraMode)
    {
        g_Camera.ProcessMouseMovement(xOffset, yOffset);
    }
}

unsigned int LoadTexture(const std::string& filePath)
{
    unsigned int TextureID;
    glGenTextures(1, &TextureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
        {
            format = GL_RED;
        }
        else if (nrComponents == 3)
        {
            format = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path " << filePath << "\n";
        stbi_image_free(data);
    }

    return TextureID;
}
