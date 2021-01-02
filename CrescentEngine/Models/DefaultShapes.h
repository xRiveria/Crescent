#pragma once
#include "Mesh.h"

namespace CrescentEngine
{
	//A 3D cube in the range [-0.5f, 0.5f]. Vertices are hard-coded.
	class Cube : public Mesh
	{
	public:
		Cube();
	};

	//A 1x1 tesselated plane made of xSegments & ySegments & 2 Triangles. Underlying topology is triangle strips.
	//The plane's default orientation is flat on the ground or spanned alongside the X and Z axis, with the positive Y axis being the normal.		
	class Plane : public Mesh
	{
	public:
		Plane(unsigned int xSegments, unsigned int ySegments);
	};

	//A 3D torus (donut) mesh characterized by its 2 radii: inner hole and ring's radius and the number of horizontal/vertical ring segments. 
	class Torus : public Mesh
	{
	public:
		Torus(float r1, float r2, unsigned int lod1, unsigned int lod2);
	};

	//A 3D unit sphere characterized by its number of horizontal (xSegments) and vertical (ySegments) rings.
	class Sphere : public Mesh
	{
	public:
		Sphere(unsigned int xSegments, unsigned int ySegments);
	};

	//A 2D quad that fills the screen with a depth position of 0.0f. Primarily intended for screen-space effects as its default NDC completely fills the screen. This usage is not
	//enforced however and can be used for other intents and purposes.
	class Quad : public Mesh
	{
	public:

		Quad(); //Defaults to dimensions of (-1, 1).
		Quad(float quadWidth, float quadHeight);
	};

	//A 2D unit circle mesh characterized by its number of edge segments (outer edges) and the number of inner ring segments.
	class Circle : public Mesh
	{
	public:
		Circle(unsigned int edgeSegments, unsigned int ringSegments);
	};

	//A line strip of specified width/height. Segments equal the number of 2-triangle sets while width equals the width of the line. Useful for line systems that require a filled line strip.
	class LineStrip : public Mesh
	{
	public:
		LineStrip(float width, unsigned int segments);
	};

}