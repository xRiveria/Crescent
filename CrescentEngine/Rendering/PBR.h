#pragma once
#include "Renderer.h"
#include "../Rendering/Texture.h"

namespace Crescent
{
	class PBR
	{
	public:
		PBR(Renderer* renderer);

		void SetSkyCapture(PBRCapture* pbrEnvironment);
		PBRCapture* GetSkyCapture();
		void RenderProbes(); //Renders all reflection/irradiance probes for visualization/debugging.
		PBRCapture* ProcessEquirectangular(Texture2D* environmentMap);
	};
}
