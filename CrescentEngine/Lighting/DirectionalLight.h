#pragma once
#include <glm/glm.hpp>

namespace Crescent
{
	class RenderTarget;

	/*
		Light container object for any 3D directional light source. Directional light types support shadow casting, holding a reference to the RenderTarget and the
		relevant Light Space View Projection Matrix used for its shadow map generation.
	*/

	class DirectionalLight
	{
	public:
		glm::vec3 m_LightDirection = glm::vec3(0.0f);
		glm::vec3 m_LightColor = glm::vec3(1.0f);
		float m_LightIntensity = 1.0f;

		bool m_ShadowCastingEnabled = true;
		RenderTarget* m_ShadowMapRenderTarget = nullptr;
		glm::mat4 m_LightSpaceViewProjectionMatrix = glm::mat4(1.0f);
	};
}