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

	//A 2D quad that's screen-facing with a depth position of 0.0. Its main use is primarily intended for screen-space effects as its default NDC coordinates completely
	//fill the screen. However, this usage is not enforced and they can be used for other intents and purposes.
	class Quad : public Mesh
	{
	public:
		Quad(); //Defaults to dimensions (1, 1).
		Quad(float quadWidth, float quadHeight);
	};
}