#pragma once
#include <glm/glm.hpp>

namespace Crescent
{
	class Texture;

	enum Shader_Type
	{
		Shader_Type_Boolean,
		Shader_Type_Integer,
		Shader_Type_Float,
		Shader_Type_Sampler1D,
		Shader_Type_Sampler2D,
		Shader_Type_Sampler3D,
		Shader_Type_Vector2,
		Shader_Type_Vector3,
		Shader_Type_Vector4,
		Shader_Type_Matrix2,
		Shader_Type_Matrix3,
		Shader_Type_Matrix4
	};

	struct Uniform
	{
		Shader_Type m_UniformType;
		std::string m_UniformName;
		int m_UniformSize;
		unsigned int m_UniformLocation;
	};

	struct VertexAttribute
	{
		Shader_Type m_AttributeType;
		std::string m_AttributeName;
		int m_AttributeSize;
		unsigned int m_AttributeLocation;
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
			Texture* m_Texture;
		};

		UniformSamplerValue() {}
	};
}
