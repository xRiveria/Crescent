#pragma once
#include "TextureCube.h"
#include <glm/glm.hpp>

namespace CrescentEngine
{
	//Container object for holding all per-environemnt specific pre-computed PBR data.
	struct PBRCapture
	{
		TextureCube* m_Irradiance = nullptr;
		TextureCube* m_Prefiltered = nullptr;

		glm::vec3 m_Position;
		float m_Radius;
	};
}