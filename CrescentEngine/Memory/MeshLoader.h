#pragma once
#include "Scene/SceneNode.h"
#include "../Rendering/Renderer.h"
#include "../Models/Mesh.h"
#include "../Rendering/Material.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace CrescentEngine
{
	class MeshLoader
	{
	public:
		static void ClearMeshLibrary();
		static SceneNode* LoadMesh(Renderer* renderer, const std::string& filePath, bool setDefaultMaterial = true);

	private:
		static SceneNode* ProcessNode(Renderer* renderer, aiNode* assimpNode, const aiScene* assimpScene, const std::string& fileDirectory, bool setDefaultMaterial);
		static Mesh* ParseMesh(aiMesh* assimpMesh, const aiScene* assimpScene, glm::vec3& outMin, glm::vec3& outMax);
		
		//Creates a material for each mesh and extracts each unique texture from the mesh.	
		static Material* ParseMaterial(Renderer* renderer, aiMaterial* assimpMaterial, const aiScene* assimpScene, const std::string& fileDirectory);

		static std::string ProcessPath(aiString* assimpPath, const std::string& fileDirectory);

	private:
		static std::vector<Mesh*> m_MeshLibrary; //Keeps track of all loaded mesh.
	};
}