#include "CrescentPCH.h"
#include "FlyCamera.h"
#include "glm/gtx/compatibility.hpp"

namespace Crescent
{
	FlyCamera::FlyCamera(glm::vec3 cameraPosition, glm::vec3 forwardDirection, glm::vec3 upDirection) : Camera(cameraPosition, forwardDirection, upDirection)
	{
		m_CameraYaw = -90.0f;

		m_ForwardDirection = forwardDirection;
		m_WorldUp = m_UpDirection;
		m_TargetPosition = cameraPosition;
	}

	void FlyCamera::Update(float deltaTime)
	{ 
		//Slowly interpolate to target position each frame given some damping factor.
		//This gives smooth camera movement that fades out the closer we are to our target.
		m_CameraPosition = glm::lerp(m_CameraPosition, m_TargetPosition, glm::clamp(deltaTime * m_Damping, 0.0f, 1.0f)); 
		m_CameraYaw = glm::lerp(m_CameraYaw, m_TargetYaw, glm::clamp(deltaTime * m_Damping * 2.0f, 0.0f, 0.0f));  
		m_CameraPitch = glm::lerp(m_CameraPitch, m_TargetPitch, glm::clamp(deltaTime * m_Damping * 2.0f, 0.0f, 1.0f));

		//Calculate new cartesian basis vectors from yaw/pitch pair.
		glm::vec3 newForward;
		newForward.x = cos(0.0174533 * m_CameraPitch) * cos(0.0174533 * m_CameraYaw);
		newForward.y = sin(0.0174533 * m_CameraPitch);
		newForward.z = cos(0.0174533 * m_CameraPitch) * sin(0.0174533 * m_CameraYaw);
		m_ForwardDirection = glm::normalize(newForward);

		m_RightDirection = glm::normalize(glm::cross(m_ForwardDirection, m_WorldUp));
		m_UpDirection = glm::normalize(glm::cross(m_RightDirection, m_ForwardDirection));

		// calculate the new view matrix
		UpdateViewMatrix();
	}

	void FlyCamera::InputKey(float deltaTime, CameraMovement movementDirection)
	{
		float speed = m_MovementSpeed * deltaTime;

		if (movementDirection == CameraForward)
		{
			m_TargetPosition = m_TargetPosition + m_ForwardDirection * speed;
		}
		else if (movementDirection == CameraBack)
		{
			m_TargetPosition = m_TargetPosition - m_ForwardDirection * speed;
		}
		else if (movementDirection == CameraLeft)
		{
			m_TargetPosition = m_TargetPosition - m_RightDirection * speed;
		}
		else if (movementDirection == CameraRight)
		{
			m_TargetPosition = m_TargetPosition + m_RightDirection * speed;
		}
		else if (movementDirection == CameraUp)
		{
			m_TargetPosition = m_TargetPosition + m_WorldUp * speed;
		}
		else if (movementDirection == CameraDown)
		{
			m_TargetPosition = m_TargetPosition - m_WorldUp * speed;
		}
	}

	void FlyCamera::InputMouse(float deltaX, float deltaY)
	{
		float xMovement = deltaX * m_MouseSensitivity;
		float yMovement = deltaY * m_MouseSensitivity;

		m_TargetYaw += xMovement;
		m_TargetPitch += yMovement;

		//Prevents calculating the length of the null vector.
		if (m_TargetYaw == 0.0f)
		{
			m_TargetYaw = 0.01f;
		}
		if (m_TargetPitch == 0.0f)
		{
			m_TargetPitch = 0.01f;
		}

		//Its not allowed to move the pitch above or below 90 degrees. Else, the current world up direction would break our LookAt calculation.
		if (m_TargetPitch > 89.0f)
		{
			m_TargetPitch = 89.0f;
		}

		if (m_TargetPitch < -89.0f)
		{
			m_TargetPitch = -89.0f;
		}
	}

	void FlyCamera::InputScroll(float deltaX, float deltaY)
	{
		m_MovementSpeed = glm::clamp(m_MovementSpeed + deltaY * 1.0f, 1.0f, 25.0f);
		m_Damping = glm::clamp(m_Damping + deltaX * 0.5f, 1.0f, 25.0f);
	}
}