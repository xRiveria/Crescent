#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Crescent
{
	//Defines several possible options for camera movement. Used as abstractions to stay away from windows specific input methods.
	enum CameraMovement
	{
		CameraForward,
		CameraBack,
		CameraLeft,
		CameraRight,
		CameraUp,
		CameraDown
	};

	/*
		Our basic root camera. Only does relevant camera calculations with manual forced direction setters. 
		This camera should only be used in code and not respond to user input - thats what the derived cameras are for.
	*/

	class Camera
	{
	public:
		Camera();
		Camera(glm::vec3 cameraPosition, glm::vec3 forwardDirection, glm::vec3 upDirection);

		void Update(float deltaTime);

		void SetPerspectiveMatrix(float fieldOfView, float aspectRatio, float nearClip, float farClip);
		void SetOrthographicMatrix(float leftBound, float rightBound, float topBound, float bottomBound, float nearClip, float farClip);

		void UpdateViewMatrix();

	public:
		float m_CameraYaw;
		float m_CameraPitch;

		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;

		glm::vec3 m_CameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_ForwardDirection = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 m_UpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_RightDirection = glm::vec3(1.0f, 0.0f, 0.0f);

		float m_FieldOfView;
		float m_AspectRatio;
		float m_NearClip;
		float m_FarClip;
		bool m_IsPerspectiveCamera;
	};
}