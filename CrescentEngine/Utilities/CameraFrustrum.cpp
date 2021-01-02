#include "CrescentPCH.h"
#include "CameraFrustrum.h"

namespace Crescent
{
	CameraFrustrum::CameraFrustrum()
	{
	}
	void CameraFrustrum::Update(Camera* camera)
	{
	}
	bool CameraFrustrum::Intersect(glm::vec3 point)
	{
		return false;
	}
	bool CameraFrustrum::Intersect(glm::vec3 point, float radius)
	{
		return false;
	}
	bool CameraFrustrum::Intersect(glm::vec3 boundingBoxMinimum, glm::vec3 boundingBoxMaximum)
	{
		return false;
	}
}