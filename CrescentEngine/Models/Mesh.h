#pragma once
#include "Core.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include "../Shading/Shader.h"
#include <algorithm>
#include <assimp/scene.h>
#include <map>
#include "BoneMapper.h"

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
	};

	struct MeshTexture
	{
		unsigned int id;  //Assigned Memory Location
		std::string type; //Type of texture eg. Specular/Diffuse
		std::string path;
	};

	struct MeshAnimation
	{
		MeshAnimation(aiAnimation* animation, std::string animationName, float animationTimeInSeconds, int animationIndex) : m_Animation(animation),
			m_AnimationName(animationName), m_AnimationTimeInSeconds(animationTimeInSeconds), m_AnimationIndex(animationIndex)
		{

		}

		aiAnimation* m_Animation;
		std::string m_AnimationName;
		int m_AnimationIndex;
		float m_AnimationTimeInSeconds;
	};

	enum Topology
	{
		Triangles,
		TriangleStrips
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

		//Skeletal Animations
		void RecursivelyUpdateBoneMatrices(int animation_id, aiNode* node, glm::mat4 transform, double ticks);
		glm::mat4 InterpolateTranslationMatrix(aiVectorKey* keys, uint32_t n, double ticks);
		glm::mat4 InterpolateRotationMatrix(aiQuatKey* keys, uint32_t n, double ticks);
		glm::mat4 InterpolateScalingMatrix(aiVectorKey* keys, uint32_t n, double ticks);

		void AddBone(int vertexWeightID, int id, float weight)
		{
			int i;
			for (i = 0; i < 8; i++)
			{
				if (m_BoneWeights[i].first == 0) //Find empty slot.
				{
					break;
				}

				if (i >= 8) //If we have more bones than supported.
				{
					CrescentError("Model has more bones than supported.");
				}
			}

			//Once empty slot is found, assign here.
			m_BoneIDs[vertexWeightID] = std::pair<int, int>(i, id);
			m_BoneWeights[vertexWeightID] = std::pair<int, float>(i, weight);
		}

	public:
		Topology m_Topology = Triangles;

		std::vector<glm::vec3> m_Positions;
		std::vector<glm::vec2> m_UV;
		std::vector<glm::vec3> m_Normals;
		std::vector<glm::vec3> m_Tangents;
		std::vector<glm::vec3> m_Bitangents;	
		std::vector<std::pair<int, int>> m_BoneIDs;
		std::vector<std::pair<int, float>> m_BoneWeights;
		

		std::vector<unsigned int> m_Indices;

		//Skeletal Animations
		std::vector<glm::mat4> m_BoneMatrices, m_BoneOffsets;
		int m_CurrentlyPlayingAnimationIndex;
		std::vector<MeshAnimation*> m_Animations; //Stores a vector of animations mapped to an index.
		std::map<std::pair<uint32_t, std::string>, uint32_t> m_AnimationChannelMap;
		BoneMapper m_BoneMapper;

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

