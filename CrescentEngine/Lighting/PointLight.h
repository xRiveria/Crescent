#pragma once
#include <glm/glm.hpp>

namespace Crescent
{
	/*
		Light container object for any 3D point light source. The range of these point lights are solely determined by a radius volume which is used for their
		frustrum culling and attenuation properties. The attenuation is calculated based on a slightly tweaked point light attenuation equation.
	*/

	class PointLight
	{
	public:
		glm::vec3 m_LightPosition = glm::vec3(0.0f);
		glm::vec3 m_LightColor = glm::vec3(0.0f);

		float m_LightIntensity = 1.0f;
		float m_LightRadius = 1.0f;
		bool m_IsLightVisible = true;
		bool m_RenderMesh = false;
	};
}
