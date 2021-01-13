#pragma once
#include "../Shading/TextureCube.h"
#include <glm/glm.hpp>

namespace Crescent
{
	//Container object for holding all per-environment specific pre-computed PBR data.
	struct EnvironmentalPBR
	{
		glm::vec3 m_Position;
		float m_Radius;

		TextureCube* m_IrradianceTextureCube = nullptr;
		TextureCube* m_PrefilteredTextureCube = nullptr;
	};
}