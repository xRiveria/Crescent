#pragma once
#include <string>
#include "Mesh.h"
#include "../LearnShader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
//Note that a model can contain multiple meshes with multiple textures.

namespace CrescentEngine
{
	class Model
	{
	public:
		Model(const std::string& filePath)
		{
			LoadModel(filePath);
		}

		void Draw(LearnShader& shader);

	private:
		//Model Data
		std::vector<Mesh> m_Meshes;
		std::vector<Texture> m_TexturesLoaded;
		std::string m_FileDirectory;

		void LoadModel(const std::string& filePath);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName);
	};
}