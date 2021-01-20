#pragma once
#include <string>
#include <glm/glm.hpp>
#include "ShaderUtilities.h"

namespace Crescent
{
	/*
		Shader object for quickly creating and using a GPU shader program. When compiling/linking a shader object frokm source code, all vertex attributes and shaders
		are extracted for saving unnecessary additional CPU-GPU roundtrip cycles.
	*/
	class Shader
	{
	public:
		Shader();
		Shader(const std::string& shaderName, std::string vertexShaderCode, std::string fragmentShaderCode);

		void LoadShader(const std::string& shaderName, std::string vertexShaderCode, std::string fragmentShaderCode);
		void UseShader();
		bool HasUniform(const std::string& uniformName);

		void DeleteShader();

		void SetUniformFloat(const std::string name, float value);
		void SetUniformInteger(std::string name, int value);
		void SetUniformBool(std::string name, bool value);
		void SetUniformVector2(std::string name, glm::vec2 value);
		void SetUniformVector3(std::string name, glm::vec3 value);
		void SetUniformMat4(std::string name, glm::mat4 value);
		void SetUniformVectorArray(std::string name, int size, const std::vector<glm::vec3>& values);
		void SetUniformVectorMat4(std::string identifier, std::vector<glm::mat4> value);

		inline unsigned int GetShaderID() const { return m_ShaderID; }

	public:
		//Defunct
		Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		void CreateShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

	private:
		void CheckCompileErrors(unsigned int shader, std::string type);
		
		//Retrieves uniform location from pre-stored uniform locations and reports an error if a non-uniform is set.
		int RetrieveUniformLocation(const std::string& uniformName);
		unsigned int m_ShaderID;

	private:
		std::string m_ShaderName;
		std::vector<Uniform> m_Uniforms;
		std::vector<VertexAttribute> m_Attributes;
	};
}