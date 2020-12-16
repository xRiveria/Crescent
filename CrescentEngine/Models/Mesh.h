#pragma once
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include "../LearnShader.h"
#include <algorithm>

namespace CrescentEngine
{
	struct Vertex  //Defined for each vertice on a mesh.
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		int BoneIDs[8];
		float BoneWeights[8];

		Vertex()
		{
			std::fill(BoneIDs, BoneIDs + 8, 0);
			std::fill(BoneWeights, BoneWeights + 8, 0);
		}

		void AddBone(int id, float weight)
		{
			int i;
			for (i = 0; i < 8; i++)
			{
				if (BoneWeights[i] == 0) //Find empty slot.
				{
					break;
				}
				
				if (i >= 8) //If we have more bones than supported.
				{
					std::cout << "Too Many Bones!";
					std::terminate(); 
				}
			}

			//Once empty slot is found, assign here.
			BoneWeights[i] = weight;
			BoneIDs[i] = id;
		}
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
		unsigned int vertexArrayObject, vertexBufferObject, indexBufferObject, boneBufferObject;
		void SetupMesh();
	};
}

