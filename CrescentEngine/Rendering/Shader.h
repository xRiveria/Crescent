#pragma once
#include <string>
#include <glm/glm.hpp>

namespace CrescentEngine
{
	class Shader
	{
	public:
		Shader() {}
		Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		Shader(const std::string& shaderName, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, std::vector<std::string> defines = std::vector<std::string>());
		void CreateShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		void UseShader();
		void UnbindShader();
		void DeleteShader();

		void SetUniformFloat(const std::string& name, float value) const;
		void SetUniformInteger(const std::string& name, int value) const;
		void SetUniformBool(const std::string& name, bool value) const;
		void SetUniformVector3(const std::string& name, const glm::vec3& value);
		void SetUniformMat4(const std::string& name, const glm::mat4& value);
		void SetUniformVectorMat4(const std::string& identifier, const std::vector<glm::mat4>& value);

		inline unsigned int GetShaderID() const { return m_ShaderID; }

	private:
		void CheckCompileErrors(unsigned int shader, std::string type);
		unsigned int m_ShaderID;
	};
}