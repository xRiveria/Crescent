#include "CrescentPCH.h"
#include "Mesh.h"
#include "GL/glew.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

namespace Crescent
{
	Mesh::Mesh()
	{

	}

	Mesh::Mesh(std::vector<glm::vec3> positions, std::vector<unsigned int> indices)
	{
		m_Positions = positions;
		m_Indices = indices;
	}

	Mesh::Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<unsigned int> indices)
	{
		m_Positions = positions;
		m_UV = uv;
		m_Indices = indices;
	}

	Mesh::Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<unsigned int> indices)
	{
		m_Positions = positions;
		m_UV = uv;
		m_Normals = normals;
		m_Indices = indices;
	}

	Mesh::Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices)
	{
		m_Positions = positions;
		m_UV = uv;
		m_Normals = normals;
		m_Tangents = tangents;
		m_Bitangents = bitangents;
		m_Indices = indices;
	}

	void Mesh::FinalizeMesh(bool interleaved)
	{
		//Initialize IDs if not configured before.
		if (!m_VertexArrayID)
		{
			glGenVertexArrays(1, &m_VertexArrayID);
			glGenBuffers(1, &m_VertexBufferID);
			glGenBuffers(1, &m_IndexBufferID);
		}

		//Preprocess buffer data.
		std::vector<float> bufferData;

		if (interleaved)
		{
			for (int i = 0; i < m_Positions.size(); i++)
			{
				bufferData.push_back(m_Positions[i].x);
				bufferData.push_back(m_Positions[i].y);
				bufferData.push_back(m_Positions[i].z);
				if (m_UV.size() > 0)
				{
					bufferData.push_back(m_UV[i].x);
					bufferData.push_back(m_UV[i].y);
				}
				if (m_Normals.size() > 0)
				{
					bufferData.push_back(m_Normals[i].x);
					bufferData.push_back(m_Normals[i].y);
					bufferData.push_back(m_Normals[i].z);
				}
				if (m_Tangents.size() > 0)
				{
					bufferData.push_back(m_Tangents[i].x);
					bufferData.push_back(m_Tangents[i].y);
					bufferData.push_back(m_Tangents[i].z);
				}
				if (m_Bitangents.size() > 0)
				{
					bufferData.push_back(m_Bitangents[i].x);
					bufferData.push_back(m_Bitangents[i].y);
					bufferData.push_back(m_Bitangents[i].z);
				}				
			}
		}
		else
		{
			//If any of the float arrays are empty, data won't be filled by them.
			for (int i = 0; i < m_Positions.size(); i++)
			{
				bufferData.push_back(m_Positions[i].x);
				bufferData.push_back(m_Positions[i].y);
				bufferData.push_back(m_Positions[i].z);
			}
			for (int i = 0; i < m_UV.size(); i++)
			{
				bufferData.push_back(m_UV[i].x);
				bufferData.push_back(m_UV[i].y);
			}
			for (int i = 0; i < m_Normals.size(); i++)
			{
				bufferData.push_back(m_Normals[i].x);
				bufferData.push_back(m_Normals[i].y);
				bufferData.push_back(m_Normals[i].z);
			}
			for (int i = 0; i < m_Tangents.size(); i++)
			{
				bufferData.push_back(m_Tangents[i].x);
				bufferData.push_back(m_Tangents[i].y);
				bufferData.push_back(m_Tangents[i].z);
			}
			for (int i = 0; i < m_Bitangents.size(); i++)
			{
				bufferData.push_back(m_Bitangents[i].x);
				bufferData.push_back(m_Bitangents[i].y);
				bufferData.push_back(m_Bitangents[i].z);
			}
		}

		//Configure vertex attributes only if vertex data size is more than 0.
		glBindVertexArray(m_VertexArrayID);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, (bufferData.size() * sizeof(float) + (m_BoneIDs.size() * sizeof(int)) + (m_BoneWeights.size() * sizeof(float))), &bufferData[0], GL_STATIC_DRAW);
		//Only fill the index buffer if the index array is not empty.
		if (m_Indices.size() > 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);
		}
		if (interleaved)
		{
			//Calculate stride from number of non-empty vertex attribute arrays. Remember that stride is the total amount of information owned by a single vertex.
			size_t stride = 3 * sizeof(float); //Positions
			if (m_UV.size() > 0) stride += 2 * sizeof(float);
			if (m_Normals.size() > 0) stride += 3 * sizeof(float);
			if (m_Tangents.size() > 0) stride += 3 * sizeof(float);
			if (m_Bitangents.size() > 0) stride += 3 * sizeof(float);
			//if (m_BoneIDs.size() > 0) stride += 8 * sizeof(int);
			//if (m_BoneWeights.size() > 0) stride += 8 * sizeof(float);

			size_t offset = 0;
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
			offset += 3 * sizeof(float);
			if (m_UV.size() > 0)
			{
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
				offset += 2 * sizeof(float);
			}
			if (m_Normals.size() > 0)
			{
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
				offset += 3 * sizeof(float);
			}
			if (m_Tangents.size() > 0)
			{
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
				offset += 3 * sizeof(float);
			}
			if (m_Bitangents.size() > 0)
			{
				glEnableVertexAttribArray(4);
				glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
				offset += 3 * sizeof(float);
			}
			/*
			if (m_BoneIDs.size() > 0)
			{
				glEnableVertexAttribArray(5);
				glVertexAttribIPointer(5, 4, GL_INT, stride, (GLvoid*)(offset));
				offset += 4 * sizeof(int);

				glEnableVertexAttribArray(6);
				glVertexAttribIPointer(6, 4, GL_INT, stride, (GLvoid*)(offset));
				offset += 4 * sizeof(int);
			}

			if (m_BoneWeights.size() > 0)
			{
				glEnableVertexAttribArray(7);
				glVertexAttribPointer(7, 4, GL_FLOAT, false, stride, (GLvoid*)(offset));
				offset += 4 * sizeof(float);

				glEnableVertexAttribArray(8);
				glVertexAttribPointer(8, 4, GL_FLOAT, false, stride, (GLvoid*)(offset));
				offset += 4 * sizeof(float);
			}
			*/
		}
		else
		{
			size_t offset = 0;
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
			offset += m_Positions.size() * sizeof(float);

			if (m_UV.size() > 0)
			{
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
				offset += m_UV.size() * sizeof(float);
			}
			if (m_Normals.size() > 0)
			{
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
				offset += m_Normals.size() * sizeof(float);
			}
			if (m_Tangents.size() > 0)
			{
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
				offset += m_Tangents.size() * sizeof(float);
			}
			if (m_Bitangents.size() > 0)
			{
				glEnableVertexAttribArray(4);
				glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
				offset += m_Bitangents.size() * sizeof(float);
			}

			/*
			if (m_BoneIDs.size() > 0)
			{
				glEnableVertexAttribArray(5);
				glVertexAttribIPointer(5, 4, GL_INT, 0, (GLvoid*)(offset));

				glEnableVertexAttribArray(6);
				glVertexAttribIPointer(6, 4, GL_INT, 0, (GLvoid*)(offset));
				offset += m_BoneIDs.size() * sizeof(int);
			}

			if (m_BoneWeights.size() > 0)
			{
				glEnableVertexAttribArray(7);
				glVertexAttribPointer(7, 4, GL_FLOAT, false, 0, (GLvoid*)(offset));

				glEnableVertexAttribArray(8);
				glVertexAttribPointer(8, 4, GL_FLOAT, false, 0, (GLvoid*)(offset));
				offset += m_BoneWeights.size() * sizeof(float);
			}
			*/
		}
		glBindVertexArray(0);
	}	

	//==================================================================================================================

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTexture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		SetupMesh();
	}

	void Mesh::Draw(Shader& shader, bool renderShadowMap, unsigned int shadowMapTextureID)
	{
		//Bind appropriate textures.
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;

		//Draw Mesh
		glBindVertexArray(vertexArrayObject);

		shader.UseShader();

		if (!renderShadowMap)
		{
			for (unsigned int i = 0; i < textures.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i); //Activate proper texture unit before binding.
				//Retrieve texture number (the N in diffuse_textureN)
				std::string number;
				std::string name = textures[i].type;

				if (name == "texture_diffuse")
				{
					number = std::to_string(diffuseNr++); //Convert to string and return before incrementing.
				}
				else if (name == "texture_specular")
				{
					number = std::to_string(specularNr++);
				}
				else if (name == "texture_normal")
				{
					number = std::to_string(normalNr++);
				}
				else if (name == "texture_height")
				{
					number = std::to_string(heightNr++);
				}

				shader.UseShader();
				glUniform1i(glGetUniformLocation(shader.GetShaderID(), (name + number).c_str()), i);
				glBindTexture(GL_TEXTURE_2D, textures[i].id);
			}
		}

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		for (int i = 0; i < 32; i++)
		{
			if (i == 3)
			{
				continue;
			}
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glBindVertexArray(0);
	}

	void Mesh::SetupMesh()
	{
		glGenVertexArrays(1, &vertexArrayObject);

		glGenBuffers(1, &vertexBufferObject);
		glGenBuffers(1, &indexBufferObject);

		glBindVertexArray(vertexArrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		//Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		//Vertex Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		//Vertex Texture Coodinates
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		//Vertex Tangents
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

		//Vertex Bitangents
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
		
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, BoneIDs) + 0 * sizeof(int)));

		glEnableVertexAttribArray(6);
		glVertexAttribIPointer(6, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, BoneIDs) + 4 * sizeof(int)));

		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, false, sizeof(Vertex), (void*)(offsetof(Vertex, BoneWeights) + 0 * sizeof(float)));

		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, false, sizeof(Vertex), (void*)(offsetof(Vertex, BoneWeights) + 4 * sizeof(float)));
		
		glBindVertexArray(0);
	}

	void Mesh::RecursivelyUpdateBoneMatrices(int animationIndex, aiNode* node, glm::mat4 transform, double ticks)
	{
		static auto mat4_from_aimatrix4x4 = [](aiMatrix4x4 matrix) -> glm::mat4 {
			glm::mat4 res;
			for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) res[j][i] = matrix[i][j];
			return res;
		};

		std::string node_name = node->mName.C_Str();
		auto animation = m_Animations[animationIndex]->m_Animation;
		glm::mat4 current_transform;

		if (m_AnimationChannelMap.count(std::pair<uint32_t, std::string>(animationIndex, node_name)))
		{
			uint32_t channel_id = m_AnimationChannelMap[std::pair<uint32_t, std::string>(animationIndex, node_name)];
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
			RecursivelyUpdateBoneMatrices(animationIndex, node->mChildren[i], transform * current_transform, ticks);
		}
	}

	glm::mat4 Mesh::InterpolateTranslationMatrix(aiVectorKey* keys, uint32_t n, double ticks)
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

	glm::mat4 Mesh::InterpolateRotationMatrix(aiQuatKey* keys, uint32_t n, double ticks)
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

	glm::mat4 Mesh::InterpolateScalingMatrix(aiVectorKey* keys, uint32_t n, double ticks)
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
