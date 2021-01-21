#include "CrescentPCH.h"
#include "MeshLoader.h"
#include "../Scene/SceneEntity.h"
#include "../Models/Mesh.h"
#include "../Rendering/Resources.h"
#include "../Shading/Material.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Rendering/Renderer.h"

namespace Crescent
{
    std::vector<Mesh*> MeshLoader::m_MeshStore = std::vector<Mesh*>();
    // --------------------------------------------------------------------------------------------
    void MeshLoader::ClearMeshStore()
    {
        for (unsigned int i = 0; i < MeshLoader::m_MeshStore.size(); ++i)
        {
            delete MeshLoader::m_MeshStore[i];
        }
    }
    // --------------------------------------------------------------------------------------------
    SceneEntity* MeshLoader::LoadMesh(Renderer* rendererContext, const std::string& filePath, bool setDefaultMaterial)
    {
        CrescentLoad("Loading mesh: " + filePath + ".");
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            CrescentError("Assimp failed to load model at path: " + filePath);
            return nullptr;
        }

        std::string directory = filePath.substr(0, filePath.find_last_of("/"));

        CrescentLoad("Succesfully loaded: " + filePath + ".");

        return MeshLoader::ProcessNode(rendererContext, scene->mRootNode, scene, directory, setDefaultMaterial);
    }

    SceneEntity* MeshLoader::ProcessNode(Renderer* rendererContext, aiNode* aiNode, const aiScene* aiScene, const std::string& fileDirectory, bool setDefaultMaterial)
    {
        //Note that we allocate memory ourselves and pass memory responsibility to calling resource manager. 
        //The resource manager is responsible for holding the scene entity pointer and deleting where appropriate.
        SceneEntity* node = new SceneEntity(aiNode->mName.C_Str(), 0);

        for (unsigned int i = 0; i < aiNode->mNumMeshes; ++i)
        {
            aiMesh* assimpMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
            aiMaterial* assimpMat = aiScene->mMaterials[assimpMesh->mMaterialIndex];
            Mesh* mesh = MeshLoader::ParseMesh(assimpMesh, aiScene);
            Material* material = nullptr;
            if (setDefaultMaterial)
            {
                material = MeshLoader::ParseMaterial(rendererContext, assimpMat, aiScene, fileDirectory);
            }

            //If we only have one mesh, this entity itself contains the mesh/material.
            if (aiNode->mNumMeshes == 1)
            {
                node->m_Mesh = mesh;
                if (setDefaultMaterial)
                {
                    node->m_Material = material;
                }
            }

            //Otherwise, the meshes are considered on equal depth of its children
            else
            {
                SceneEntity* child = new SceneEntity(aiScene->mMeshes[i]->mName.C_Str(), 0);
                child->m_Mesh = mesh;
                child->m_Material = material;
                node->AddChildEntity(child);
            }
        }

        //Also recursively parse this node's children 
        for (unsigned int i = 0; i < aiNode->mNumChildren; ++i)
        {
            node->AddChildEntity(MeshLoader::ProcessNode(rendererContext, aiNode->mChildren[i], aiScene, fileDirectory, setDefaultMaterial));
        }

        return node;
    }
    
    Mesh* MeshLoader::ParseMesh(aiMesh* aiMesh, const aiScene* aiScene)
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec3> bitangents;
        std::vector<unsigned int> indices;

        positions.resize(aiMesh->mNumVertices);
        normals.resize(aiMesh->mNumVertices);
        if (aiMesh->mNumUVComponents > 0)
        {
            uv.resize(aiMesh->mNumVertices);
            tangents.resize(aiMesh->mNumVertices);
            bitangents.resize(aiMesh->mNumVertices);
        }
        //We assume a constant of 3 vertex indices per face as we always triangulate in Assimp's post-processing step. Otherwise, you'll want transform this to a more flexible scheme.
        indices.resize(aiMesh->mNumFaces * 3);

        for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
        {
            positions[i] = glm::vec3(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
            normals[i] = glm::vec3(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
            if (aiMesh->mTextureCoords[0])
            {
                uv[i] = glm::vec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);

            }
            if (aiMesh->mTangents)
            {
                tangents[i] = glm::vec3(aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z);
                bitangents[i] = glm::vec3(aiMesh->mBitangents[i].x, aiMesh->mBitangents[i].y, aiMesh->mBitangents[i].z);
            }
        }
        for (unsigned int f = 0; f < aiMesh->mNumFaces; ++f)
        {
            //We know we're always working with triangles due to the Triangulate option.
            for (unsigned int i = 0; i < 3; ++i)
            {
                indices[f * 3 + i] = aiMesh->mFaces[f].mIndices[i];
            }
        }

        Mesh* mesh = new Mesh;
        mesh->m_Positions = positions;
        mesh->m_UV = uv;
        mesh->m_Normals = normals;
        mesh->m_Tangents = tangents;
        mesh->m_Bitangents = bitangents;
        mesh->m_Indices = indices;
        mesh->m_Topology = Triangles;
        mesh->FinalizeMesh(true);

        if (aiScene->HasAnimations())
        {
            ProcessMeshAnimations(aiScene, aiMesh, mesh);

            /*
            auto mat4_from_aimatrix4x4 = [](aiMatrix4x4 matrix) -> glm::mat4
            {
                glm::mat4 res;
                for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) res[j][i] = matrix[i][j];
                return res;
            };

            for (int i = 0; aiMesh->mNumBones; i++)
            {
                aiBone* bone = aiMesh->mBones[i];
                uint32_t boneID = mesh->m_BoneMapper.Name(bone->mName.C_Str());

                mesh->m_BoneOffsets.resize(std::max(boneID + 1, (uint32_t)mesh->m_BoneOffsets.size()));
                mesh->m_BoneOffsets[boneID] = mat4_from_aimatrix4x4(bone->mOffsetMatrix);

                for (int j = 0; bone->mNumWeights; j++)
                {
                   // auto weight = bone->mWeights[j];
                    //mesh->AddBone(weight.mVertexId, boneID, weight.mWeight);
                }
            }

            mesh->m_BoneMatrices.resize(mesh->m_BoneMapper.RetrieveTotalBones());
            */
        }

        //Store newly generated mesh in globally stored mesh store for memory de-allocation when a clean is required.
        MeshLoader::m_MeshStore.push_back(mesh);

        return mesh;
    }

    void MeshLoader::ProcessMeshAnimations(const aiScene* aiScene, aiMesh* aiMesh, Mesh* mesh)
    {
        mesh->m_Animations.clear();
        mesh->m_AnimationChannelMap.clear();
        if (aiScene->HasAnimations())
        {
            for (int i = 0; i < aiScene->mNumAnimations; i++)
            {
                aiAnimation* animation = aiScene->mAnimations[i];
                float animationTime = aiScene->mAnimations[i]->mDuration / aiScene->mAnimations[i]->mTicksPerSecond;
                std::string animationName = aiScene->mAnimations[i]->mName.C_Str();
                mesh->m_Animations.push_back(new MeshAnimation(animation, animationName, animationTime, i));

                //Channel Mapping
                for (int j = 0; j < animation->mNumChannels; j++) //Each channelm is actually a bone with all of its transformations. 
                {
                    aiNodeAnim* channel = animation->mChannels[j];
                    mesh->m_AnimationChannelMap[std::pair<uint32_t, std::string>(i, channel->mNodeName.C_Str())] = j; //Creates a map of the animation index and its node channel name mapped to the channel index.
                }
            }
        }
        return;      
    }

    Material* MeshLoader::ParseMaterial(Renderer* rendererContext, aiMaterial* aiMaterial, const aiScene* aiScene, const std::string& fileDirectory)
    {
        //Create a unique default material for each loaded mesh.     
        Material* material;

        //Check if diffuse texture has alpha, if so: make alpha blend material.
        aiString file;
        aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
        std::string diffPath = std::string(file.C_Str());
        bool alpha = false;
        if (diffPath.find("_alpha") != std::string::npos)
        {
            std::cout << "Has Alpha!";
            material = rendererContext->CreateMaterial(); ///
            alpha = true;
        }
        else  // Else, we make a default deferred material.
        {
            material = rendererContext->CreateMaterial();
        }

         /*We use a PBR metallic/roughness workflow.
            - aiTextureType_DIFFUSE:   Albedo
            - aiTextureType_NORMALS:   Normal
            - aiTextureType_SPECULAR:  Metallic
            - aiTextureType_SHININESS: Roughness
            - aiTextureType_AMBIENT:   AO (Ambient Occlusion)
            - aiTextureType_EMISSIVE:  Emissive
        */

        if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            // We only load the first of the list of diffuse textures as we don't really care about meshes with multiple diffuse layers; same holds for other texture types.
            aiString file;
            aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
            std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);
            //We name the texture itself the same as the filename as to reduce naming conflicts while still only loading unique textures.
            Texture* texture = Resources::LoadTexture(fileName, fileName, GL_TEXTURE_2D, alpha ? GL_RGBA : GL_RGB, true);
            if (texture)
            {
                material->SetShaderTexture("TexAlbedo", texture, 3);
            }
        }

        if (aiMaterial->GetTextureCount(aiTextureType_DISPLACEMENT) > 0)
        {
            aiString file;
            aiMaterial->GetTexture(aiTextureType_DISPLACEMENT, 0, &file);
            std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);

            Texture* texture = Resources::LoadTexture(fileName, fileName);
            if (texture)
            {
                material->SetShaderTexture("TexNormal", texture, 4);
            }
        }

        if (aiMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0)
        {
            aiString file;
            aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
            std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);

            Texture* texture = Resources::LoadTexture(fileName, fileName);
            if (texture)
            {
                material->SetShaderTexture("TexMetallic", texture, 5);
            }
        }

        if (aiMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
        {
            aiString file;
            aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &file);
            std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);

            Texture* texture = Resources::LoadTexture(fileName, fileName);
            if (texture)
            {
                material->SetShaderTexture("TexRoughness", texture, 6);
            }
        }

        if (aiMaterial->GetTextureCount(aiTextureType_AMBIENT) > 0)
        {
            aiString file;
            aiMaterial->GetTexture(aiTextureType_AMBIENT, 0, &file);
            std::string fileName = MeshLoader::ProcessPath(&file, fileDirectory);

            Texture* texture = Resources::LoadTexture(fileName, fileName);
            if (texture)
            {
                material->SetShaderTexture("TexAO", texture, 7);
            }
        }

        return material;
    }

    std::string MeshLoader::ProcessPath(aiString* aPath, std::string directory)
    {
        std::string path = std::string(aPath->C_Str());

        //Parse path directly if path contains "/" indicating it is an absolute path; otherwise parse as relative.
        if (path.find(":/") == std::string::npos || path.find(":\\") == std::string::npos)
        {
            path = directory + "/" + path;
        }
        return path;
    }
}