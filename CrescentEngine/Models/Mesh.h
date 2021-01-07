#pragma once
#include "Core.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include "../Shading/Shader.h"
#include <algorithm>

namespace Crescent
{
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

	struct MeshTexture
	{
		unsigned int id;  //Assigned Memory Location
		std::string type; //Type of texture eg. Specular/Diffuse
		std::string path;
	};

	/*
		Basic Mesh class. A mesh in its simplest form is purely a list of vertices with some added functionality for easily setting up the hardware configuration
		relevant for rendering.
	*/

	class Mesh
	{
	public:
		//Here, we support multiple ways of initializing a mesh. 
		Mesh();
		Mesh(std::vector<glm::vec3> positions, std::vector<unsigned int> indices);
		Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<unsigned int> indices);
		Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<unsigned int> indices);
		Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices);

		void FinalizeMesh(bool interleaved = true); //Preprocess buffer data as interleaved or seperate when specified. 

		//Retrieves
		unsigned int RetrieveVertexArrayID() const { return m_VertexArrayID; }

	public:
		std::vector<glm::vec3> m_Positions;
		std::vector<glm::vec2> m_UV;
		std::vector<glm::vec3> m_Normals;
		std::vector<glm::vec3> m_Tangents;
		std::vector<glm::vec3> m_Bitangents;

		std::vector<unsigned int> m_Indices;

	private:
		unsigned int m_VertexArrayID = 0;
		unsigned int m_VertexBufferID = 0;
		unsigned int m_IndexBufferID = 0;

	public:
		//Defunct
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTexture> textures);
		void Draw(Shader& shader, bool renderShadowMap, unsigned int shadowMapTextureID);
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<MeshTexture> textures;

		//Defunct
		unsigned int vertexArrayObject, vertexBufferObject, indexBufferObject;
		void SetupMesh();

	};
}

