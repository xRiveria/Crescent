#pragma once
#include <string>
#include <glm/glm.hpp>
#include "ShaderUtilities.h"

namespace Crescent
{
	/*
		Shader object for quickly creating and using a GPU shader object. When compiling/linking a shader from source code, all vertex attributes and uniforms are extracted for saving
		unnecessary additional CPU/GPU roundtrip times.	
	*/

	class Shader
	{
	public:
		Shader();
		Shader(const std::string& shaderName, std::string vertexShaderCode, std::string fragmentShaderCode, std::vector<std::string> defines = std::vector<std::string>());
		
		void LoadShader(const std::string& shaderName, std::string vertexShaderCode, std::string fragmentShaderCode, std::vector<std::string> defines = std::vector<std::string>());
		void UseShader();
		bool HasUniform(const std::string& name);

		void SetUniformInteger(const std::string& name, int value);
		void SetUniformFloat(const std::string& name, float value);
		void SetUniformBool(const std::string& name, bool value);
		void SetUniformVector3(const std::string& name, const glm::vec3& value);
		void SetUniformMat4(const std::string& name, const glm::mat4& value);
		void SetUniformVectorMat4(const std::string& name, const std::vector<glm::mat4>& value);

		unsigned int RetrieveShaderID() const { return m_ShaderID; }

		//Defunct
		Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		void CreateShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		void UnbindShader();
		void DeleteShader();

	public:
		std::string m_ShaderName;
		std::vector<Uniform> m_Uniforms;
		std::vector<VertexAttribute> m_Attributes;

	private:
		//Retrieves uniform location from pre-stored uniform locations and reports an error if a non-uniform is set.
		int RetrieveUniformLocation(const std::string& name);

	private:
		unsigned int m_ShaderID;

		//Defunct
		void CheckCompileErrors(unsigned int shader, std::string type);
	};
}