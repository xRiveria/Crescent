#include "CrescentPCH.h"
#include "Core/Renderer.h"
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

float deltaTime = 0.0f;	// Time between current frame and last frame.
float lastFrame = 0.0f; // Time of last frame.

//Camera
Camera g_Camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool firstMove = true;
float lastX = 400;
float lastY = 300;

//Settings
unsigned int m_ScreenWidth = 800;
unsigned int m_ScreenHeight = 600;
float visibleValue = 0.1f;

//Lighting
glm::vec3 lightPosition = { 1.2f, 1.0f, 2.0f };

//This is a callback function that is called whenever a window is resized.
void FramebufferResizeCallback(GLFWwindow* window, int windowWidth, int windowHeight)
{
    glViewport(0, 0, windowWidth, windowHeight);
    m_ScreenHeight = windowHeight;
    m_ScreenWidth = windowWidth; 
}

void ImGuiSetup();
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void ProcessInput(GLFWwindow* window);
void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

int main()
{
    RendererAbstractor::Renderer::InitializeSelectedRenderer(RendererAbstractor::Renderer::API::OpenGL);

    /// ===== Hello Window =====

    glfwInit();
    //Refer to https://www.glfw.org/docs/latest/window.html#window_hints for a list of window hints avaliable here. This goes into the first value of glfwWindowHint.
    //We are essentially setting the value of the enum (first value) to the second value.
    //Here, we are telling GLFW that 3.3 is the OpenGL version we want to use. This allows GLFW to make the proper arrangements when creating the OpenGL context.
    //Thus, when users don't have the proper OpenGL version on his or her computer that is lower than 3.3, GLFW will fail to run, crash or display undefined behavior. 
    //We thus set the major and minor version to both 3 in this case.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //We also tell GLFW that we explicitly want to use the core profile. This means we will get access to a smaller subset of OpenGL features without backwards compatible features we no longer need.
    //On Mac OS X, you need to add "glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);" for it to work.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   
    //Creates a Window Object. This window object holds all the windowing data and is required by most of GLFW's other functions. 
    //The "glfwCreateWindow()" function requires the window width and height as its first two arguments respectively.
    //The third argument allows us to create a name for the window which I've called "OpenGL".
    //We can ignore the last 2 parameters. The function returns a GLFWwindow object that we will later need for other GLFW operations.
    GLFWwindow* window = glfwCreateWindow(m_ScreenWidth, m_ScreenHeight, "OpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW Window! \n";
        glfwTerminate();
        return -1;
    }

    //We tell GLFW to make the context of our window the main context on the current thread.

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //Sets a callback to a viewport resize function everytime we resize our window
 
    glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);


    //Initializes GLEW. 
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error!" << std::endl;
    }
    glEnable(GL_DEPTH_TEST);

    LearnShader lightingShader("Resources/Shaders/VertexShader.shader", "Resources/Shaders/FragmentShader.shader");
    LearnShader lightCubeShader("Resources/Shaders/LightVertexShader.shader", "Resources/Shaders/LightFragmentShader.shader");

    //Because OpenGL works in 3D space, we render a 2D triangle with each vertex having a Z coordinate of 0.0. This way, the depth of the triangle remains the same, making it look like its 2D. 
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,  
             0.5f,  0.5f, -0.5f,  
            -0.5f,  0.5f, -0.5f,  
            -0.5f, -0.5f, -0.5f,  

            -0.5f, -0.5f,  0.5f,  
             0.5f, -0.5f,  0.5f,  
             0.5f,  0.5f,  0.5f,  
             0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f,  0.5f,  
            -0.5f, -0.5f,  0.5f,  

            -0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f, -0.5f,  
            -0.5f, -0.5f, -0.5f,  
            -0.5f, -0.5f, -0.5f,  
            -0.5f, -0.5f,  0.5f,  
            -0.5f,  0.5f,  0.5f,  

             0.5f,  0.5f,  0.5f,  
             0.5f,  0.5f, -0.5f,  
             0.5f, -0.5f, -0.5f,  
             0.5f, -0.5f, -0.5f,  
             0.5f, -0.5f,  0.5f,  
             0.5f,  0.5f,  0.5f,  

            -0.5f, -0.5f, -0.5f,  
             0.5f, -0.5f, -0.5f,  
             0.5f, -0.5f,  0.5f,  
             0.5f, -0.5f,  0.5f,  
            -0.5f, -0.5f,  0.5f,  
            -0.5f, -0.5f, -0.5f,  

            -0.5f,  0.5f, -0.5f,  
             0.5f,  0.5f, -0.5f,  
             0.5f,  0.5f,  0.5f,  
             0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f, -0.5f,  
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Light Source VBO

    unsigned int lightVertexArrayObject;
    glGenVertexArrays(1, &lightVertexArrayObject);
    glBindVertexArray(lightVertexArrayObject);
    //We only need to bind to the VBO, the container's VBO data also ready contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    //Set the vertex attribute.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


#if Texture
    unsigned int textureData1, textureData2;
    glGenTextures(1, &textureData1);
    glBindTexture(GL_TEXTURE_2D, textureData1);
    //Sets the texture wrapping and filtering options on the currently bound texture object. 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //When objects are zoomed in aka scaled up, we interpolate from a combination of nearest texels to the fragment.
   
 //Load and generate the texture.
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("Resources/Textures/Container.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load textures! \n";
    }
    stbi_image_free(data);

    glGenTextures(1, &textureData2);
    glBindTexture(GL_TEXTURE_2D, textureData2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data2 = stbi_load("Resources/Textures/AwesomeFace.png", &width, &height, &nrChannels, 0);
    if (data2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data2);
    
#endif

    //unsigned int colorUniformLocation = glGetUniformLocation(shaderProgram, "ourColor");
    //glUseProgram(shaderProgram);
    //ourShader.UseShader(); //Always activate shaders before setting uniforms.
    //ourShader.SetUniformInteger("texture1", 0);
    // ourShader.SetUniformInteger("texture2", 1);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();
        ProcessInput(window); //Process key input events.

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
        lightingShader.UseShader();
        lightingShader.SetUniformVector3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
        lightingShader.SetUniformVector3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

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
        cubeObjectMatrix = glm::rotate(modelMatrix, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f)); //Rotation on the X-axis and Y axis.
        
        lightingShader.SetUniformMat4("model", cubeObjectMatrix);
        glBindVertexArray(vertexArrayObject);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //Draw our Lamp Object

        lightCubeShader.UseShader();
        lightCubeShader.SetUniformMat4("projection", projectionMatrix);
        lightCubeShader.SetUniformMat4("view", viewMatrix);

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, lightPosition);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f)); //A smaller cube.     
        lightCubeShader.SetUniformMat4("model", modelMatrix);

        glBindVertexArray(lightVertexArrayObject);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        glfwSwapBuffers(window);
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

}

void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    g_Camera.ProcessMouseScroll(yOffset);
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

    g_Camera.ProcessMouseMovement(xOffset, yOffset);
}
