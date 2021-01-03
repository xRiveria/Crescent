#pragma once
#include "Shader.h"
#include "Texture.h"
#include "TextureCube.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
#include "ShaderUtilities.h"

namespace Crescent
{
	/* 
		A material object represents all relevant render states required for rendering a mesh. 
		This includes shader parameters, texture samplers and GL states.
		A material object is required for rendering any scene node. 
		The renderer itself holds a list of common material defaults/templates for deriving or creating new materials. 
	*/
	
	enum MaterialType
	{
		Material_Default,
		Material_Custom,
		Material_PostProcess
	};

	class Material
	{
	public:
		Material();
		Material(Shader* shader);

		//Shaders
		Shader* RetrieveMaterialShader() const { return m_Shader; }
		void SetMaterialShader(Shader* shader) { m_Shader = shader; }

		Material Copy();

		//Uniforms
		std::map<std::string, UniformValue>* GetUniforms();
		std::map<std::string, UniformSamplerValue>* GetSamplerUniforms();

		void SetShaderBool(const std::string& uniformName, const bool& value);
		void SetShaderInt(const std::string& uniformName, const int& value);
		void SetShaderFloat(const std::string& uniformName, const float& value);
		void SetShaderTexture(const std::string& uniformName, Texture2D* value, unsigned int textureUnit = 0);
		void SetShaderTextureCube(const std::string& uniformName, TextureCube* value, unsigned int textureUnit = 0);
		void SetShaderVector2(const std::string& uniformName, const glm::vec2& value);
		void SetShaderVector3(const std::string& uniformName, const glm::vec3& value);
		void SetShaderVector3(const std::string& uniformName, const glm::vec4& value);
		void SetShaderMat2(const std::string& uniformName, const glm::mat2& value);
		void SetShaderMat3(const std::string& uniformName, const glm::mat3& value);
		void SetShaderMat4(const std::string& uniformName, const glm::mat4& value);

	public:
		MaterialType m_MaterialType = Material_Custom;
		glm::vec4 m_Color = glm::vec4(1.0f);

		//Depth State
		bool m_DepthTestEnabled = true;
		bool m_DepthWritingEnabled = true;
		GLenum m_DepthTestComparisonFactor = GL_LESS;

		//Face Culling State
		bool m_FaceCullingEnabled = true;
		GLenum m_CulledFace = GL_BACK;
		GLenum m_CullWindingOrder = GL_CCW;

		//Blending
		bool m_BlendingEnabled = false;
		GLenum m_BlendSource = GL_ONE; //Pre-multipled alpha.
		GLenum m_BlendDestination = GL_ONE_MINUS_SRC_ALPHA;
		GLenum m_BlendEquation = GL_FUNC_ADD;

		//Shadow State
		bool m_ShadowCasting = true;
		bool m_ShadowReceiving = true;

	private:
		Shader* m_Shader;
		std::map<std::string, UniformValue> m_Uniforms;
		std::map<std::string, UniformSamplerValue> m_SamplerUniforms;
	};
}