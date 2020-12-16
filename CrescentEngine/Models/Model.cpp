#include "CrescentPCH.h"
#include "Model.h"
#include "GL/glew.h"
#include "stb_image/stb_image.h"
#include "glm/gtc/matrix_transform.hpp"

namespace CrescentEngine
{
	unsigned int TextureFromFile(const std::string& path, const std::string& directory);

	void Model::Draw(uint32_t animationID, double time, bool loop, LearnShader& shader)
	{
		RecursivelyUpdateBoneMatrices(animationID, m_ModelScene->mRootNode, glm::mat4(1), time * m_ModelScene->mAnimations[animationID]->mTicksPerSecond);
		for (unsigned int i = 0; i < m_Meshes.size(); i++)
		{		
			m_Meshes[i].Draw(shader);
		}
	}

	void Model::Draw(LearnShader& shader)
	{
		for (unsigned int i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].Draw(shader);
		}
	}

	//Should consider creating a Shader automagically for each Model loaded.
	void Model::LoadModel(const std::string& filePath)
	{
		m_ModelScene = m_Importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!m_ModelScene || m_ModelScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_ModelScene->mRootNode)
		{
			std::cout << "Error::Assimp::" << m_Importer.GetErrorString() << "\n";
			return;
		}

		m_AnimationChannelMap.clear();
		for (int i = 0; i < m_ModelScene->mNumAnimations; i++)
		{
			auto animation = m_ModelScene->mAnimations[i];

			for (int j = 0; j < animation->mNumChannels; j++)
			{
				auto channel = animation->mChannels[j];
				m_AnimationChannelMap[std::pair<uint32_t, std::string>(i, channel->mNodeName.C_Str())] = j; //Create a map of the animation index, its node channel name, and the channel's index.
			}
		}

		m_FileDirectory = filePath.substr(0, filePath.find_last_of('/'));
		ProcessNode(m_ModelScene->mRootNode); //Pass root node to process, which can possbily contain more child nodes.
		m_BoneMatrices.resize(m_BoneMapper.RetrieveTotalBones());
	}

	void Model::ProcessNode(aiNode* node)
	{
		//Process all node's meshes if any.
		for (unsigned int i = 0; i < node->mNumMeshes; i++) //Remember that each index in a mesh indices set points to a specific mesh located in the scene object.
		{
			aiMesh* mesh = m_ModelScene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) //Do it all over again if the node contains children.
		{
			ProcessNode(node->mChildren[i]);
		}
	}

	Mesh Model::ProcessMesh(const aiMesh* mesh)
	{
		auto mat4_from_aimatrix4x4 = [](aiMatrix4x4 matrix) -> glm::mat4 {
			glm::mat4 res;
			for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) res[j][i] = matrix[i][j];
			return res;
		};

		//Data we must fill.
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		//Walk through each of the mesh's vertices.
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector; //We declare a placeholder vector 
			// positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			// normals
			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}

			//Texture Coordinates
			if (mesh->mTextureCoords[0]) //Does the mesh contain texture coordinatres?
			{
				glm::vec2 vec;
				//A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't use models where a vertex can have mutiple texture coordinates so we always take the first set of (0).
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}

		//Now, walk through each of the mesh's faces and retrieve the corresponding vertex indices. 
		//Assimp's interface defines each mesh as having an array of faces, where each face represents a single primitive, which in our case (due to aiProcess_Triangulate option) are always triangles.
		//A face contains the indices of the vertices we need to draw in what order for its primitives.
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		aiMaterial* material = m_ModelScene->mMaterials[mesh->mMaterialIndex];

		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		
		for (int i = 0; i < mesh->mNumBones; i++)
		{
			auto bone = mesh->mBones[i];
			auto id = m_BoneMapper.Name(bone->mName.C_Str());

			m_BoneOffsets.resize(std::max(id + 1, (uint32_t)m_BoneOffsets.size())); 
			m_BoneOffsets[id] = mat4_from_aimatrix4x4(bone->mOffsetMatrix);

			for (int j = 0; j < bone->mNumWeights; j++)
			{
				auto weight = bone->mWeights[j];
				vertices[weight.mVertexId].AddBone(id, weight.mWeight);
			}
		}
		
		return Mesh(vertices, indices, textures);
	}

	//Iterates over all the texture locations of the given texture type, retrieve the texture's file location and then loads and generates the texture.
	std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString string;
			material->GetTexture(type, i, &string);

			bool skip = false;
			for (unsigned int j = 0; j < m_TexturesLoaded.size(); j++)
			{
				if (std::strcmp(m_TexturesLoaded[j].path.data(), string.C_Str()) == 0)
				{
					textures.push_back(m_TexturesLoaded[j]);
					skip = true;
					break;
				}
			}
			
			if (!skip)
			{
				Texture texture;
				texture.id = TextureFromFile(string.C_Str(), this->m_FileDirectory);
				texture.type = typeName;
				texture.path = string.C_Str();
				textures.push_back(texture);
				m_TexturesLoaded.push_back(texture); //Add to loaded textures;
			}
		}
		return textures;
	}

	unsigned int TextureFromFile(const std::string& path, const std::string& directory)
	{
		std::string filename = path;
		filename = directory + '/' + filename;
		std::cout << filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}

	void Model::RecursivelyUpdateBoneMatrices(int animation_id, aiNode* node, glm::mat4 transform, double ticks)
	{
		static auto mat4_from_aimatrix4x4 = [](aiMatrix4x4 matrix) -> glm::mat4 {
			glm::mat4 res;
			for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) res[j][i] = matrix[i][j];
			return res;
		};

		std::string node_name = node->mName.C_Str();
		auto animation = m_ModelScene->mAnimations[animation_id];
		glm::mat4 current_transform;

		if (m_AnimationChannelMap.count(std::pair<uint32_t, std::string>(animation_id, node_name))) 
		{
			uint32_t channel_id = m_AnimationChannelMap[std::pair<uint32_t, std::string>(animation_id, node_name)];
			auto channel = animation->mChannels[channel_id];

			// translation matrix
			glm::mat4 translation_matrix = InterpolateTranslationMatrix(channel->mPositionKeys, channel->mNumPositionKeys, ticks);
			// rotation matrix
			glm::mat4 rotation_matrix = InterpolateRotationMatrix(channel->mRotationKeys, channel->mNumRotationKeys, ticks);
			// scaling matrix
			glm::mat4 scaling_matrix = InterpolateScalingMatrix(channel->mScalingKeys, channel->mNumScalingKeys, ticks);

			current_transform = translation_matrix * rotation_matrix * scaling_matrix;
		}
		else 
		{
			current_transform = mat4_from_aimatrix4x4(node->mTransformation);
		}
		
		if (m_BoneMapper.RetrieveBoneLibrary().count(node_name)) 
		{
			uint32_t i = m_BoneMapper.RetrieveBoneLibrary()[node_name];
			m_BoneMatrices[i] = transform * current_transform * m_BoneOffsets[i];
		}

		for (int i = 0; i < node->mNumChildren; i++) 
		{
			RecursivelyUpdateBoneMatrices(animation_id, node->mChildren[i], transform * current_transform, ticks);
		}
	}

	glm::mat4 Model::InterpolateTranslationMatrix(aiVectorKey* keys, uint32_t n, double ticks)
	{
		static auto mat4_from_aivector3d = [](aiVector3D vector) -> glm::mat4 {
			return glm::translate(glm::mat4(1), glm::vec3(vector.x, vector.y, vector.z));
		};
		if (n == 0) return glm::mat4(1);
		if (n == 1) return mat4_from_aivector3d(keys->mValue);
		if (ticks <= keys[0].mTime) return mat4_from_aivector3d(keys[0].mValue);
		if (keys[n - 1].mTime <= ticks) return mat4_from_aivector3d(keys[n - 1].mValue);

		aiVectorKey anchor;
		anchor.mTime = ticks;
		auto right_ptr = std::upper_bound(keys, keys + n, anchor, [](const aiVectorKey& a, const aiVectorKey& b) {
			return a.mTime < b.mTime;
			});
		auto left_ptr = right_ptr - 1;

		float factor = (ticks - left_ptr->mTime) / (right_ptr->mTime - left_ptr->mTime);
		return mat4_from_aivector3d(left_ptr->mValue * (1.0f - factor) + right_ptr->mValue * factor);
	}

	glm::mat4 Model::InterpolateRotationMatrix(aiQuatKey* keys, uint32_t n, double ticks)
	{
		static auto mat4_from_aiquaternion = [](aiQuaternion quaternion) -> glm::mat4 {
			auto rotation_matrix = quaternion.GetMatrix();
			glm::mat4 res(1);
			for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) res[j][i] = rotation_matrix[i][j];
			return res;
		};
		if (n == 0) return glm::mat4(1);
		if (n == 1) return mat4_from_aiquaternion(keys->mValue);
		if (ticks <= keys[0].mTime) return mat4_from_aiquaternion(keys[0].mValue);
		if (keys[n - 1].mTime <= ticks) return mat4_from_aiquaternion(keys[n - 1].mValue);

		aiQuatKey anchor;
		anchor.mTime = ticks;
		auto right_ptr = std::upper_bound(keys, keys + n, anchor, [](const aiQuatKey& a, const aiQuatKey& b) {
			return a.mTime < b.mTime;
			});
		auto left_ptr = right_ptr - 1;

		double factor = (ticks - left_ptr->mTime) / (right_ptr->mTime - left_ptr->mTime);
		aiQuaternion out;
		aiQuaternion::Interpolate(out, left_ptr->mValue, right_ptr->mValue, factor);
		return mat4_from_aiquaternion(out);
	}

	glm::mat4 Model::InterpolateScalingMatrix(aiVectorKey* keys, uint32_t n, double ticks)
	{
		static auto mat4_from_aivector3d = [](aiVector3D vector) -> glm::mat4 {
			return glm::scale(glm::mat4(1), glm::vec3(vector.x, vector.y, vector.z));
		};
		if (n == 0) return glm::mat4(1);
		if (n == 1) return mat4_from_aivector3d(keys->mValue);
		if (ticks <= keys[0].mTime) return mat4_from_aivector3d(keys[0].mValue);
		if (keys[n - 1].mTime <= ticks) return mat4_from_aivector3d(keys[n - 1].mValue);

		aiVectorKey anchor;
		anchor.mTime = ticks;
		auto right_ptr = std::upper_bound(keys, keys + n, anchor, [](const aiVectorKey& a, const aiVectorKey& b) {
			return a.mTime < b.mTime;
			});
		auto left_ptr = right_ptr - 1;

		float factor = (ticks - left_ptr->mTime) / (right_ptr->mTime - left_ptr->mTime);
		return mat4_from_aivector3d(left_ptr->mValue * (1.0f - factor) + right_ptr->mValue * factor);
	}
}