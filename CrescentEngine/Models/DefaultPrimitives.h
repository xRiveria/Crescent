#pragma once
#include "Mesh.h"

namespace Crescent
{
	//A 3D cube in the range [-0.5 and 0.5]. Vertices are hard coded.
	class Cube : public Mesh
	{
	public:
		Cube();
	};

	class Circle : public Mesh
	{
	public:
		Circle(unsigned int edgeSegments, unsigned int ringSegments);
	};
}