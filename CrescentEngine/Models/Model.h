#pragma once
#include <string>
#include "Mesh.h"
#include "../LearnShader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include <glm/glm.hpp>
#include "BoneMapper.h"
#include <utility>

namespace CrescentEngine
{
	class Model
	{
	public:
		Model(const std::string& filePath)
		{
			LoadModel(filePath);
		}

		void Draw(uint32_t animationID, double time, bool loop, LearnShader& shader);
		void Draw(LearnShader& shader);
		std::vector<glm::mat4> m_BoneMatrices, m_BoneOffsets;

	private:
		//Model Data
		std::vector<Mesh> m_Meshes;
		std::vector<Texture> m_TexturesLoaded;
		std::string m_FileDirectory;

		//Bones
		BoneMapper m_BoneMapper;

		std::map<std::pair<uint32_t, std::string>, uint32_t> m_AnimationChannelMap;

		void RecursivelyUpdateBoneMatrices(int animation_id, aiNode* node, glm::mat4 transform, double ticks);

		static glm::mat4 InterpolateTranslationMatrix(aiVectorKey* keys, uint32_t n, double ticks);
		static glm::mat4 InterpolateRotationMatrix(aiQuatKey* keys, uint32_t n, double ticks);
		static glm::mat4 InterpolateScalingMatrix(aiVectorKey* keys, uint32_t n, double ticks);

		//Assimp
		Assimp::Importer m_Importer;
		const aiScene* m_ModelScene;

		void LoadModel(const std::string& filePath);
		void ProcessNode(aiNode* node);
		Mesh ProcessMesh(const aiMesh* mesh);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName);	
	};
}