#pragma once
#include "Core.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include "../Rendering/Shader.h"
#include <algorithm>

namespace CrescentEngine
{
	enum Topology
	{
		POINTS,
		LINES,
		LINE_STRIP,
		Triangles,
		TRIANGLE_STRIP,
		TRIANGLE_FAN,
	};

	struct Vertex  //Defined for each vertice on a mesh.
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

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
					CrescentError("Model has more bones than supported.");
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
		Mesh();
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
		void Draw(Shader& shader, bool renderShadowMap, unsigned int shadowMapTextureID);

	public:
		// commits all buffers and attributes to the GPU driver
		void Finalize(bool interleaved = true);

	public:
		//New Structure
		std::vector<glm::vec3> m_Positions;
		std::vector<glm::vec2> m_UV;
		std::vector<glm::vec3> m_Normals;
		std::vector<glm::vec3> m_Tangents;
		std::vector<glm::vec3> m_Bitangents;
		std::vector<unsigned int> m_Indices;
		Topology m_Topology = Triangles;

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

	private:
		//Render Data
		unsigned int vertexArrayObject, vertexBufferObject, indexBufferObject;
		void SetupMesh();
	};
}

