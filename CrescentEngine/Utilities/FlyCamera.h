#pragma once
#include "Camera.h"

namespace Crescent
{
	/*
		Derivation of the base camera with support for fly-through motions. Think of WASD forward/right type of movement, combined with strafing and free yaw/pitch camera rotation.
	*/

	class FlyCamera : public Camera
	{
	public:
		FlyCamera(glm::vec3 cameraPosition, glm::vec3 forwardDirection = glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3 upDirection = glm::vec3(0.0f, 1.0f, 0.0f));

		virtual void Update(float deltaTime);

		virtual void InputKey(float deltaTime, CameraMovement movementDirection);
		virtual void InputMouse(float deltaX, float deltaY);
		virtual void InputScroll(float deltaX, float deltaY);

	public:
		float m_MovementSpeed = 10.0f;
		float m_MouseSensitivity = 0.1f;
		float m_Damping = 5.0f;

		bool m_FirstMove = true;

	private:
		glm::vec3 m_TargetPosition;
		glm::vec3 m_WorldUp;

		float m_TargetYaw;
		float m_TargetPitch;	
	};
}