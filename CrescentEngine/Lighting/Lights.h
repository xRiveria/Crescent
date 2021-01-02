#pragma once
#include <glm/glm.hpp>
#include "../Rendering/RenderTarget.h"

namespace Crescent
{
	/*
		Light container object for any 3D point light source. Point lights have a radius that determines its range value which is used for their frustrum culling and attenuation properties.
		The attenuation value is calculated on a tweaked point light attenuation equation derived by Epic Games.
	*/

	class PointLight
	{
	public:
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_Color = glm::vec3(1.0f);
		float m_Intensity = 1.0f;
		float m_Radius = 1.0f;
		bool m_Visible = true;
		bool m_MeshRenderEnabled = false;
	};

	/*
		Light container object for any 3D directional light source. Directional light types support shadow casting, holding a reference to the RenderTarget (and the relevant light space
		view projection matrix) used for its shadow map generation.
	*/

	class DirectionalLight
	{
	public:
		glm::vec3 m_Direction = glm::vec3(0.0f);
		glm::vec3 m_Color = glm::vec3(1.0f);
		float m_Intensity = 1.0f;

		bool m_ShadowCastingEnabled = true;
		RenderTarget* m_ShadowMapRenderTarget;
		glm::mat4 m_LightSpaceViewProjectionMatrix;
	};
}