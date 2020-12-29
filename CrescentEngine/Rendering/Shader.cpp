#include "CrescentPCH.h"
#include "Shader.h"
#include "GL/glew.h"
#include <glm/gtc/type_ptr.hpp>

namespace CrescentEngine
{
	Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		CreateShaders(vertexShaderPath, fragmentShaderPath);
	}

	Shader::Shader(const std::string& shaderName, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, std::vector<std::string> defines)
	{

	}

	void Shader::CreateShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		//Retrieve the vertex/fragment source code form filepath.
		std::string vertexCode;
		std::string fragmentCode;

		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		//Ensure ifstream objects can throw exceptions.
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			//Open Files
			vShaderFile.open(vertexShaderPath);
			fShaderFile.open(fragmentShaderPath);
			std::stringstream vShaderStream, fShaderStream;
			//Read file's buffer contents into streams.
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			//Close file handlers.
			vShaderFile.close();
			fShaderFile.close();
			//Convert stream into string.
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}

		catch (std::ifstream::failure e)
		{
			std::cout << "Error::Shader::File_Not_Successfully_Read" << std::endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		//2. Compile Shaders
		unsigned int vertex, fragment;
		//Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, nullptr);
		glCompileShader(vertex);
		CheckCompileErrors(vertex, "VERTEX");

		//Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, nullptr);
		glCompileShader(fragment);
		CheckCompileErrors(fragment, "FRAGMENT");

		//Shader Program
		m_ShaderID = glCreateProgram();
		glAttachShader(m_ShaderID, vertex);
		glAttachShader(m_ShaderID, fragment);
		glLinkProgram(m_ShaderID);
		CheckCompileErrors(m_ShaderID, "PROGRAM");

		//Delete the shaders once they've been linked into our program.
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void Shader::UseShader()
	{
		glUseProgram(m_ShaderID);
	}

	void Shader::UnbindShader()
	{
		glUseProgram(0);
	}

	void Shader::DeleteShader()
	{
		glDeleteProgram(m_ShaderID);
	}

	void Shader::SetUniformFloat(const std::string& name, float value) const
	{
		unsigned int uniformLocation = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform1f(uniformLocation, value);
	}

	void Shader::SetUniformInteger(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(m_ShaderID, name.c_str()), value);
	}

	void Shader::SetUniformBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(m_ShaderID, name.c_str()), (int)value);
	}

	void Shader::SetUniformVector3(const std::string& name, const glm::vec3& value)
	{
		unsigned int uniformLocation = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform3fv(uniformLocation, 1, glm::value_ptr(value));
	}

	void Shader::SetUniformMat4(const std::string& name, const glm::mat4& value)
	{
		glUniformMatrix4fv(glGetUniformLocation(m_ShaderID, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}

	void Shader::SetUniformVectorMat4(const std::string& identifier, const std::vector<glm::mat4>& value)
	{
		auto location = glGetUniformLocation(m_ShaderID, identifier.c_str());
		glUniformMatrix4fv(location, value.size(), GL_FALSE, value_ptr(value[0]));
	}

	void Shader::CheckCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		char infoLog[1024];

		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
				std::cout << "ERROR::SHADER::COMPILATION_ERROR_OF_TYPE: " << type << "\n" << infoLog;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING:ERROR::OF::TYPE: " << type << "\n" << infoLog;
			}
		}
	}
}

