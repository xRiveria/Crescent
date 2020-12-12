#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

//Defines several possible options for camera movement. Used as abstractions to stay away from windows specific input methods.

enum CameraMovement
{
	Forward,
	Backward,
	Left,
	Right
};

//Default Camera Values
const float g_CameraYaw = -90.0f; //Horizontal rotation.
const float g_CameraPitch = 0.0f; //Vertical rotation.
const float g_CameraSensitivity = 0.1f; 
const float g_CameraSpeed = 2.5f;
const float g_CameraZoom = 45.0f;

//An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL.
class Camera
{
public:
	//Constructor with Vectors.
	Camera(glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f), float cameraYaw = g_CameraYaw, float cameraPitch = g_CameraPitch)
		: m_CameraFront(glm::vec3(0.0f, 0.0f, -1.0f)), m_CameraSpeed(g_CameraSpeed), m_MouseSensitivity(g_CameraSensitivity), m_MouseZoom(g_CameraZoom)
	{
		m_CameraPosition = cameraPosition;
		m_CameraUp = cameraUp;
		m_CameraYaw = cameraYaw;
		m_CameraPitch = cameraPitch;

		UpdateCameraVectors();
	}

	//Constructor with Scalar Values.
	Camera(float positionX, float positionY, float positionZ, float upX, float upY, float upZ, float yaw, float pitch) : m_CameraFront(glm::vec3(0.0f, 0.0f, 1.0f)), m_CameraSpeed(g_CameraSpeed), m_MouseSensitivity(g_CameraSpeed), m_MouseZoom(g_CameraZoom)
	{
		m_CameraPosition = glm::vec3(positionX, positionY, positionZ);
		m_CameraUp = glm::vec3(upX, upY, upZ);
		m_CameraYaw = yaw;
		m_CameraPitch = pitch;

		UpdateCameraVectors();
	}

	//Returns the view matrix calculated using Euler Angles and the LookAt matrix.
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(m_CameraPosition, m_CameraPosition + m_CameraFront, m_CameraUp);
	}

	//Processes input received from any keyboard-like input system. Accepts input parameter in the form of a camera defined Enum.
	void ProcessKeyboardEvents(CameraMovement direction, float deltaTime)
	{
		float velocity = m_CameraSpeed * deltaTime; //Velocity is the speed of something in a given direction.
		if (direction == CameraMovement::Forward)
		{
			m_CameraPosition += m_CameraFront * velocity;
		}

		if (direction == CameraMovement::Backward)
		{
			m_CameraPosition -= m_CameraFront * velocity;
		}

		if (direction == CameraMovement::Left)
		{
			m_CameraPosition -= m_CameraRight * velocity;
		}

		if (direction == CameraMovement::Right)
		{
			m_CameraPosition += m_CameraRight * velocity;
		}
	}

	//Processes Input received from a mouse input system. Expects the offset value in both the X and Y directions.
	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
	{
		xOffset *= m_MouseSensitivity;
		yOffset *= m_MouseSensitivity;

		m_CameraYaw += xOffset;
		m_CameraPitch += yOffset;

		//Make sure that when pitch is out of bounds, screen doesn't get flipped.
		if (constrainPitch)
		{
			if (m_CameraPitch > 89.0f)
			{
				m_CameraPitch = 89.0f;
			}
			
			if (m_CameraPitch < -89.0f)
			{
				m_CameraPitch = -89.0f;
			}
		}

		//Update front, right and up vectors using the updated Euler angles.
		UpdateCameraVectors();
	}

	//Processes Input received from a mouse scroll wheel event. Only requires input on the vertical wheel axis.
	void ProcessMouseScroll(float yOffset)
	{
		m_MouseZoom -= (float)yOffset;
		if (m_MouseZoom < 1.0f)
		{
			m_MouseZoom = 1.0f;
		}
		
		if (m_MouseZoom > 45.0f)
		{
			m_MouseZoom = 45.0f;
		}
	}

	//Calculates the front vector from the Camera's (updated) Euler angles.
	void UpdateCameraVectors()
	{
		//Calculate the new front vector.
		glm::vec3 front;
		front.x = cos(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch));
		front.y = sin(glm::radians(m_CameraPitch));
		front.z = sin(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch));
		m_CameraFront = glm::normalize(front);

		//Also recalculate the Right and Up vector.
		m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_CameraUp)); //Normalize their vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
	}


public:
	//Camera Attributes
	glm::vec3 m_CameraPosition; //Camera's position in the world.
	glm::vec3 m_CameraFront; //Direction from the target towards the camera.
	glm::vec3 m_CameraUp; //The direction of the Y axis from the camera's position.
	glm::vec3 m_CameraRight; //The direction of the positive X axis of the camera's position.

	//Euler Angles
	float m_CameraYaw;
	float m_CameraPitch;

	//Camera Options
	float m_CameraSpeed;
	float m_MouseSensitivity;
	float m_MouseZoom;	
};
