#pragma once
#include <string>
#include "Mesh.h"
#include "../Shading/Shader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include <glm/glm.hpp>
#include "BoneMapper.h"
#include <utility>
#include <optional>
#include "Window.h"

namespace Crescent
{
	class Model
	{
	public:
		Model() {}
		Model(const std::string& modelName, const std::string& filePath, Window& window)
		{
			LoadModel(modelName, filePath, window);
		}

		void LoadDiffuseTexture(const std::string& filePath);
		void LoadNormalTexture(const std::string& filePath);
		void LoadModel(const std::string& modelName, const std::string& filePath, Window& window);
		
		//Draw Animated Model
		void DrawAnimatedModel(const float& deltaTime, bool renderShadowMap, Shader& shader, unsigned int shadowMapTextureID, const glm::vec3& modelScale, const glm::vec3& modelTranslation = { 0.0f, 0.0f, 0.0f });
		
		//Draw Static Model
		void DrawStaticModel(Shader& shader, bool renderShadowMap, unsigned int shadowMapTextureID);
		void DrawStaticModel(Shader& shader, bool renderShadowMap, unsigned int shadowMapTextureID, bool temporary, const glm::vec3& transformScale = glm::vec3(0.0f), const glm::vec3& transformPosition = glm::vec3(0.0f));

				
		float RetrieveAnimationTime() const { return m_AnimationTime; }

		void RenderSettingsInEditor(glm::vec3& modelPosition, glm::vec3& modelScale);
		//glm::mat4 RetrieveModelMatrix() const { return m_ModelMatrix; }

	public:
		std::vector<glm::mat4> m_BoneMatrices, m_BoneOffsets;

	private:
		void ProcessNode(aiNode* node);
		Mesh ProcessMesh(const aiMesh* mesh);
		std::vector<MeshTexture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName);

		void RecursivelyUpdateBoneMatrices(int animation_id, aiNode* node, glm::mat4 transform, double ticks);

		static glm::mat4 InterpolateTranslationMatrix(aiVectorKey* keys, uint32_t n, double ticks);
		static glm::mat4 InterpolateRotationMatrix(aiQuatKey* keys, uint32_t n, double ticks);
		static glm::mat4 InterpolateScalingMatrix(aiVectorKey* keys, uint32_t n, double ticks);

		std::string ConvertUUIDToString() const;
		std::optional<std::string> OpenFile(const char* filter);

	private:
		//Model Data
		std::string m_ModelName = "Model";
		std::vector<Mesh> m_Meshes;
		std::vector<MeshTexture> m_TexturesLoaded;
		std::string m_FileDirectory;
		int m_TemporaryUUID = 0;
		glm::vec3 m_ModelRotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_ModelScale = { 1.0f, 1.0f, 1.0f };

		//Skeletal Animations
		int m_CurrentlyPlayingAnimation = 0;
		BoneMapper m_BoneMapper;
		std::map<std::pair<uint32_t, std::string>, uint32_t> m_AnimationChannelMap;
		float m_InternalDeltaTime = 0;

		//Animation Selection
		std::vector<std::pair<aiAnimation*, size_t>> m_Animations;
		float m_AnimationTime = 0.0f;

		//Assimp
		Assimp::Importer m_Importer;
		const aiScene* m_ModelScene;
		Window* m_WindowContext;
	};
}