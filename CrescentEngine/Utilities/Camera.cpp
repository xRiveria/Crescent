#include "CrescentPCH.h"
#include "Camera.h"
#include <glm/gtc/type_ptr.hpp>

namespace Crescent
{
	Camera::Camera()
	{

	}

	Camera::Camera(glm::vec3 cameraPosition, glm::vec3 forwardDirection, glm::vec3 upDirection) : m_CameraPosition(cameraPosition), m_ForwardDirection(forwardDirection), m_UpDirection(upDirection)
	{
		UpdateViewMatrix();
	}

	void Camera::Update(float deltaTime)
	{

	}

	void Camera::SetPerspectiveMatrix(float fieldOfView, float aspectRatio, float nearClip, float farClip)
	{
		m_IsPerspectiveCamera = true;

		m_ProjectionMatrix = glm::perspective(fieldOfView, aspectRatio, nearClip, farClip);
		m_FieldOfView = fieldOfView;
		m_AspectRatio = aspectRatio;
		m_NearClip = nearClip;
		m_FarClip = farClip;
	}

	void Camera::SetOrthographicMatrix(float leftBound, float rightBound, float topBound, float bottomBound, float nearClip, float farClip)
	{
		m_IsPerspectiveCamera = false;

		m_ProjectionMatrix = glm::ortho(leftBound, rightBound, bottomBound, topBound, nearClip, farClip);
		m_NearClip = nearClip;
		m_FarClip = farClip;
	}

	void Camera::UpdateViewMatrix()
	{
		m_ViewMatrix = glm::lookAt(m_CameraPosition, m_CameraPosition + m_ForwardDirection, m_UpDirection);
	}
}