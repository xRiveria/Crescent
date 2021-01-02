#pragma once
#include "Core.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include "../Rendering/Shader.h"
#include <algorithm>

namespace Crescent
{
	/*
		Manually define a list of topology types to avoid linking a mesh to an OpenGL topology type directly. For cross compatability purposes.
	*/

	enum Topology
	{
		Points,
		Lines,
		LineStrip,
		Triangles,
		TriangleStrip,
		TriangleFan
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
		//Support multiple ways of initializing a mesh.
		Mesh();
		Mesh(std::vector<glm::vec3> positions, std::vector<unsigned int> indices);
		Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<unsigned int> indices);
		Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<unsigned int> indices);
		Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices);

		//Commits all buffers and attributes to the GPU driver.
		void FinalizeMesh(bool interleaved = true);

	public: //Defunct
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
		void Draw(Shader& shader, bool renderShadowMap, unsigned int shadowMapTextureID);

	public:
		std::vector<glm::vec3> m_Positions;
		std::vector<glm::vec2> m_UV;
		std::vector<glm::vec3> m_Normals;
		std::vector<glm::vec3> m_Tangents;
		std::vector<glm::vec3> m_Bitangents;
		std::vector<unsigned int> m_Indices;

		Topology m_Topology = Triangles;

	public: //Defunct
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

	private:
		//void CalculateMeshNormals(bool smoothing = true);
		//void CalculateMeshTangents();

	private:
		//Render Data
		unsigned int m_VertexArrayID = 0;
		unsigned int m_VertexBufferID = 0;
		unsigned int m_IndexBufferID = 0;

	private: //Defunct
		void SetupMesh();
	};
}

