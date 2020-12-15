#pragma once
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include "../LearnShader.h"

namespace CrescentEngine
{
	struct Vertex  //Defined for each vertice on a mesh.
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	struct Texture
	{
		unsigned int id;  //Assigned Memory Location
		std::string type; //Type of texture eg. Specular/Diffuse
		std::string path;
	};

	class Mesh
	{
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
		void Draw(LearnShader& shader);

	private:
		//Render Data
		unsigned int vertexArrayObject, vertexBufferObject, indexBufferObject;
		void SetupMesh();
	};
}

