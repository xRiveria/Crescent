#include "CrescentPCH.h"
#include "MeshLoader.h"
#include "Resources.h"

namespace CrescentEngine
{
	std::vector<Mesh*> MeshLoader::m_MeshLibrary = std::vector<Mesh*>();


	void MeshLoader::ClearMeshLibrary()
	{
		for (unsigned int i = 0; i < MeshLoader::m_MeshLibrary.size(); ++i)
		{
			delete MeshLoader::m_MeshLibrary[i];
		} 
	}

	SceneNode* MeshLoader::LoadMesh(Renderer* renderer, const std::string& filePath, bool setDefaultMaterial)
	{
		CrescentInfo("Loading mesh file at: " + filePath + ".");

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			CrescentWarn("Failed to load model at path: " + filePath);
			return nullptr;
		}

		std::string directory = filePath.substr(0, filePath.find_last_of("/"));

		CrescentInfo("Successfully loaded at: " + filePath);

		return MeshLoader::ProcessNode(renderer, scene->mRootNode, scene, directory, setDefaultMaterial);
	}

	SceneNode* MeshLoader::ProcessNode(Renderer* renderer, aiNode* assimpNode, const aiScene* assimpScene, const std::string& fileDirectory, bool setDefaultMaterial)
	{
		//Note that we allocate memory ourselves and pass memory responsibility by calling resource manager. The resource manager is responsible for holding the scene node pointer and deleting where appropriate.
		SceneNode* node = new SceneNode(0);

		for (unsigned int i = 0; i < assimpNode->mNumMeshes; ++i)
		{
			glm::vec3 boundingBoxMinimum, boundingBoxMaximum;

			aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
			aiMaterial* assimpMaterial = assimpScene->mMaterials[assimpMesh->mMaterialIndex];
			Mesh* mesh = MeshLoader::ParseMesh(assimpMesh, assimpScene, boundingBoxMinimum, boundingBoxMaximum);
			Material* material = nullptr;

			if (setDefaultMaterial)
			{
				material = MeshLoader::ParseMaterial(renderer, assimpMaterial, assimpScene, fileDirectory);
			}

			//If we only have one mesh, this node itself contains the mesh/materials.
			if (assimpNode->mNumMeshes == 1)
			{
				node->m_Mesh = mesh;
				if (setDefaultMaterial)
				{
					node->m_Material = material;
				}
				node->m_BoundingBoxMinimum = boundingBoxMinimum;
				node->m_BoundingBoxMaximum = boundingBoxMaximum;
			}
			else  //Otherwise, the meshes are considered no equal depth of its children.
			{
				SceneNode* childNode = new SceneNode(0);
				childNode->m_Mesh = mesh;
				childNode->m_Material = material;
				childNode->m_BoundingBoxMinimum = boundingBoxMinimum;
				childNode->m_BoundingBoxMaximum = boundingBoxMaximum;

				node->AddChildNode(childNode);
			}
		}

		//Also, recursively parse this node's children.
		for (unsigned int i = 0; i < assimpNode->mNumChildren; ++i)
		{
			node->AddChildNode(MeshLoader::ProcessNode(renderer, assimpNode->mChildren[i], assimpScene, fileDirectory, setDefaultMaterial));
		}

		return node; 
	}

	Mesh* MeshLoader::ParseMesh(aiMesh* assimpMesh, const aiScene* assimpScene, glm::vec3& outMin, glm::vec3& outMax)
	{
		std::vector<glm::vec3> meshPositions;
		std::vector<glm::vec2> meshUVs;
		std::vector<glm::vec3> meshNormals;
		std::vector<glm::vec3> meshTangents;
		std::vector<glm::vec3> meshBitangents;
		std::vector<unsigned int> meshIndices;

		meshPositions.resize(assimpMesh->mNumVertices);
		meshNormals.resize(assimpMesh->mNumVertices);

		if (assimpMesh->mNumUVComponents > 0)
		{
			meshUVs.resize(assimpMesh->mNumVertices);
			meshTangents.resize(assimpMesh->mNumVertices);
			meshBitangents.resize(assimpMesh->mNumVertices);
		}

		//We assume a constant of 3 vertice indices per face as we always triangulate in Assimp's post-processing step; otherwise you will want to transfer this to a more flexible scheme.
		meshIndices.resize(assimpMesh->mNumFaces * 3);

		//Store minimum/maximum point in local coordinates for calculating approximate bounding box. 
		glm::vec3 pMin(99999.0);
		glm::vec3 pMax(-99999.0);

		for (unsigned int i = 0; i < assimpMesh->mNumVertices; ++i)
		{
			meshPositions[i] = glm::vec3(assimpMesh->mVertices[i].x, assimpMesh->mVertices[i].y, assimpMesh->mVertices[i].z);
			meshNormals[i] = glm::vec3(assimpMesh->mNormals[i].x, assimpMesh->mNormals[i].y, assimpMesh->mNormals[i].z);

			if (assimpMesh->mTextureCoords[0])
			{
				meshUVs[i] = glm::vec2(assimpMesh->mTextureCoords[0][i].x, assimpMesh->mTextureCoords[0][i].y);
			}

			if (assimpMesh->mTangents)
			{
				meshTangents[i] = glm::vec3(assimpMesh->mTangents[i].x, assimpMesh->mTangents[i].y, assimpMesh->mTangents[i].z);
				meshBitangents[i] = glm::vec3(assimpMesh->mBitangents[i].x, assimpMesh->mBitangents[i].y, assimpMesh->mBitangents[i].z);
			}

			if (meshPositions[i].x < pMin.x) pMin.x = meshPositions[i].x;
			if (meshPositions[i].y < pMin.y) pMin.y = meshPositions[i].y;
			if (meshPositions[i].z < pMin.z) pMin.z = meshPositions[i].z;

			if (meshPositions[i].x < pMax.x) pMax.x = meshPositions[i].x;
			if (meshPositions[i].x < pMax.y) pMax.y = meshPositions[i].y;
			if (meshPositions[i].x < pMax.z) pMax.z = meshPositions[i].z;
		}

		for (unsigned int f = 0; f < assimpMesh->mNumFaces; ++f)
		{
			//We know we are working with triangles due to the Triangulate option.
			for (unsigned int i = 0; i < 3; ++i)
			{
				meshIndices[f * 3 + i] = assimpMesh->mFaces[f].mIndices[i];
			}
		}

		Mesh* mesh = new Mesh;
		mesh->m_Positions = meshPositions;
		mesh->m_UV = meshUVs;
		mesh->m_Normals = meshNormals;
		mesh->m_Tangents = meshTangents;
		mesh->m_Bitangents = meshBitangents;
		mesh->m_Indices = meshIndices;
		mesh->m_Topology = Triangles;
		mesh->Finalize(true);

		outMin.x = pMin.x;
		outMin.y = pMin.y;
		outMin.z = pMin.z;
		outMax.x = pMax.x;
		outMax.y = pMax.y;
		outMax.z = pMax.z;

		//Stores newly generated mesh in globally stored mesh library for memory deallocation when a clean is required.
		MeshLoader::m_MeshLibrary.push_back(mesh);

		return nullptr;
	}

	Material* MeshLoader::ParseMaterial(Renderer* renderer, aiMaterial* assimpMaterial, const aiScene* assimpScene, const std::string& fileDirectory)
	{
		//Create a unique default material for each loaded mesh.
		Material* material;
		
		//Check if diffuse texture has alpha. If so, we make an alpha blending material.
		aiString file;
		assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
		std::string diffuseTexturePath = std::string(file.C_Str());
		bool alpha = false;

		if (diffuseTexturePath.find("_alpha") != std::string::npos)
		{
			material = renderer->CreateMaterial("Alpha Discard");
			alpha = true;
		}
		else	//Else, make default deffered material.
		{
			material = renderer->CreateMaterial();
		}

		/*
			About Texture Types:
			- We use a PBR metallic/roughness workflow so the loaded models are expected to have textures that conform tp the workflow: Albedo, Normal, Metallic, Roughness, AO.
			- Since Assimp makes certain assumptions regarding possible types of loaded textures, it might not directly translate to our model.
			- We thus have to make some assumptions as well which the 3D autor has to comply with if eh wants the mesh(es) to directly render with its specified textures. 
			
			- aiTextureType_DIFFUSE:	 Albedo
			- aiTextureType_NORMALS:	 Normal
			- aiTextureType_SPECULAR:	 Metallic
			- aiTextureType_SHININESS:	 Roughness
			- aiTextureType_AMBIENT:	 AO (Ambient Occlusion)
			- aiTextureType_EMISSIVE:	 Emissive	
		*/

		if (assimpMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			//We only load the first of the list of diffuse textures, we don't really care about meshes with multiple diffuse textures; same holds for other texture types.
			aiString file;
			assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
			std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);
			
			//We name the texture the same as the file name so as to reduce naming conflicts while still loading unique textures.
			Texture2D* texture = Resources::LoadTexture(fileName, fileName, GL_TEXTURE_2D, alpha ? GL_RGBA : GL_RGB, true);

			if (texture)
			{
				material->SetShaderTexture("TexAlbedo", texture, 3);
			}
		}

		if (assimpMaterial->GetTextureCount(aiTextureType_DISPLACEMENT) > 0)
		{
			aiString file;
			assimpMaterial->GetTexture(aiTextureType_DISPLACEMENT, 0, &file);
			std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);

			Texture2D* texture = Resources::LoadTexture(fileName, fileName);
			
			if (texture)
			{
				material->SetShaderTexture("TexNormal", texture, 4);
			}
		}

		if (assimpMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			aiString file;
			assimpMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
			std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);

			Texture2D* texture = Resources::LoadTexture(fileName, fileName);

			if (texture)
			{
				material->SetShaderTexture("TexMetallic", texture, 5);
			}
		}

		if (assimpMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
		{
			aiString file;
			assimpMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
			std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);

			Texture2D* texture = Resources::LoadTexture(fileName, fileName);

			if (texture)
			{
				material->SetShaderTexture("TexRoughness", texture, 6);
			}
		}

		if (assimpMaterial->GetTextureCount(aiTextureType_AMBIENT) > 0)
		{
			aiString file;
			assimpMaterial->GetTexture(aiTextureType_AMBIENT, 0, &file);
			std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);

			Texture2D* texture = Resources::LoadTexture(fileName, fileName);

			if (texture)
			{
				material->SetShaderTexture("TexAO", texture, 7);
			}
		}

		return material;
	}

	std::string MeshLoader::ProcessPath(aiString* assimpPath, const std::string& fileDirectory)
	{
		std::string filePath = std::string(assimpPath->C_Str());

		//Parse the path directory if path contains "/" indicating it is an absolute path. Otherwise, parse as a relative path.
		if (filePath.find(":/") == std::string::npos || filePath.find(":\\") == std::string::npos)
		{
			filePath = fileDirectory + "/" + filePath;
		}

		return filePath;
	}
}