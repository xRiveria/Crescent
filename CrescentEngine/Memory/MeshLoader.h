#pragma once
#include <string>
#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiString;

namespace Crescent
{
	class Renderer;
	class SceneEntity;
	class Mesh;
	class Material;

	/*
		Mesh load functionality.
	*/

	class MeshLoader
	{
	public:
		static SceneEntity* LoadMesh(Renderer* rendererContext, const std::string& filePath, bool setDefaultMaterial = true);
		static void ClearMeshStore();

	private:
		static SceneEntity* ProcessNode(Renderer* rendererContext, aiNode* aiNode, const aiScene* aiScene, const std::string& fileDirectory, bool setDefaultMaterial = true);
		static Mesh* ParseMesh(aiMesh* aiMesh, const aiScene* aiScene);
		static Material* ParseMaterial(Renderer* rendererContext, aiMaterial* aiMaterial, const aiScene* aiScene, const std::string& fileDirectory);
		static std::string ProcessPath(aiString* filePath, std::string fileDirectory);

	private:
		static std::vector<Mesh*> m_MeshStore;
	};
}