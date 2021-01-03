#pragma once
#include <glm/glm.hpp>

namespace Crescent
{
	/*
		Data container for the 3D plane equation variables in Cartesian place. A plane equation can be defined by its Normal (perpendicular to the plane) and a distance value D
		obtained from any point on the plane itself (projected onto the normal vector). 
	*/

	struct FrustrumPlane
	{
		glm::vec3 m_Normals;
		float m_Distance;

		void SetNormalDistance(glm::vec3 normal, glm::vec3 planePoint)
		{
			m_Normals = glm::normalize(normal);
			m_Distance = -glm::dot(m_Normals, planePoint);
		}

		float Distance(glm::vec3 planePoint)
		{
			return glm::dot(m_Normals, planePoint) + m_Distance;
		}
	};

	/*
		Container object managing all 6 camera frustrum planes as calculated from any Camera object.
		The Camera Frustrum allows to quickly determine (using simple collision primitives like point, sphere, box) whether an object is within the frustrum (or cross into the frustum's edge(s).
		//This gives us the option to significantly reduce draw calls for objects that are not visible. Note that the frustrum needs to be recalculated every frame. 
	*/

	class CameraFrustrum
	{
		class Camera;

	public:
		CameraFrustrum();

		void Update(Camera* camera);

		bool Intersect(glm::vec3 point);
		bool Intersect(glm::vec3 point, float radius);
		bool Intersect(glm::vec3 boundingBoxMinimum, glm::vec3 boundingBoxMaximum);

	public:
		union
		{
			FrustrumPlane Planes[6];
			struct
			{
				FrustrumPlane Left;
				FrustrumPlane Right;
				FrustrumPlane Top;
				FrustrumPlane Bottom;
				FrustrumPlane Near;
				FrustrumPlane Far;
			};
		};
	};
}