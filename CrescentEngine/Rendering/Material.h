#pragma once
#include "Shader.h"
#include "Texture.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>

namespace Crescent
{
	/* 
		A material object represents all relevant render states required for rendering a mesh. 
		This includes shader parameters, texture samplers and GL states.
		A material object is required for rendering any scene node. 
		The renderer itself holds a list of common material defaults/templates for deriving or creating new materials. 
	*/
	
	enum Shader_Type
	{
		Shader_Type_Bool,
		Shader_Type_Int,
		Shader_Type_Float,
		Shader_Type_Sampler1D,
		Shader_Type_Sampler2D,
		Shader_Type_Sampler3D,
		Shader_Type_Vec2,
		Shader_Type_Vec3,
		Shader_Type_Vec4,
		Shader_Type_Mat2,
		Shader_Type_Mat3,
		Shader_Type_Mat4
	};

	struct UniformValue
	{
		Shader_Type m_UniformType; 

		//At the moment, each element takes up the space of its largest element, which in this case is a Mat4 of 64 bytes. Better solution needed.
		union
		{
			bool m_BoolValue;
			int m_IntValue;
			float m_FloatValue;

			glm::vec2 m_Vector2Value;
			glm::vec3 m_Vector3Value;
			glm::vec4 m_Vector4Value;

			glm::mat2 m_Mat2Value;
			glm::mat3 m_Mat3Value;
			glm::mat4 m_Mat4Value;
		};

		UniformValue() {}
	};

	struct UniformSamplerValue
	{
		Shader_Type m_UniformType;
		unsigned int m_TextureUnit;

		union
		{
			Texture2D* m_Texture;
		};

		UniformSamplerValue() {}
	};

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

		//Uniforms
		std::map<std::string, UniformValue>* GetUniforms();
		std::map<std::string, UniformSamplerValue>* GetSamplerUniforms();

		void SetShaderBool(const std::string& uniformName, const bool& value);
		void SetShaderInt(const std::string& uniformName, const int& value);
		void SetShaderFloat(const std::string& uniformName, const float& value);
		void SetShaderTexture(const std::string& uniformName, Texture2D* value, unsigned int textureUnit = 0);
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