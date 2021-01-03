#pragma once
#include <string>

namespace Crescent
{
	class Texture2D;
	class TextureCube;

	enum Shader_Type
	{
		Shader_Type_Bool,
		Shader_Type_Int,
		Shader_Type_Float,
		Shader_Type_Sampler1D,
		Shader_Type_Sampler2D,
		Shader_Type_Sampler3D,
		Shader_Type_SamplerCube,
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
			TextureCube* m_TextureCube;
		};

		UniformSamplerValue() {}
	};

	struct Uniform
	{
		Shader_Type m_UniformDataType;
		std::string m_UniformName;
		int m_UniformSize;
		unsigned int m_UniformLocation;
	};

	struct VertexAttribute
	{
		Shader_Type m_VertexAttributeDataType;
		std::string m_VertexAttributeName;
		int m_VertexAttributeSize;
		unsigned int m_VertexAttributeLocation;
	};
}
