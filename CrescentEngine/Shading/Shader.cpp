#include "CrescentPCH.h"
#include "Shader.h"
#include "GL/glew.h"
#include <glm/gtc/type_ptr.hpp>

namespace Crescent
{
	Shader::Shader()
	{

	}

	Shader::Shader(const std::string& shaderName, std::string vertexShaderCode, std::string fragmentShaderCode)
	{
		LoadShader(shaderName, vertexShaderCode, fragmentShaderCode);
	}

	void Shader::LoadShader(const std::string& shaderName, std::string vertexShaderCode, std::string fragmentShaderCode)
	{
		m_ShaderName = shaderName;
		//Compile both shaders and link them.
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		m_ShaderID = glCreateProgram();

		const char* vertexShaderSourceCode = vertexShaderCode.c_str();
		const char* fragmentShaderSourceCode = fragmentShaderCode.c_str();

		glShaderSource(vertexShader, 1, &vertexShaderSourceCode, nullptr);
		glShaderSource(fragmentShader, 1, &fragmentShaderSourceCode, nullptr);

		glCompileShader(vertexShader);
		glCompileShader(fragmentShader);

		int status;
		char log[1024];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
		if (!status)
		{
			glGetShaderInfoLog(vertexShader, 1024, NULL, log);
			CrescentInfo("Vertex shader compilation error at: " + shaderName + "!\n" + std::string(log));
		}

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
		if (!status)
		{
			glGetShaderInfoLog(fragmentShader, 1024, NULL, log);
			CrescentInfo("Fragment shader compilation error at: " + shaderName + "!\n" + std::string(log));
		}

		glAttachShader(m_ShaderID, vertexShader);
		glAttachShader(m_ShaderID, fragmentShader);
		glLinkProgram(m_ShaderID);

		glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &status);
		if (!status)
		{
			glGetProgramInfoLog(m_ShaderID, 1024, NULL, log);
			CrescentInfo("Shader program linking error: \n" + std::string(log));

			throw std::runtime_error("Shader linker error.");
		}
		
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		//Query the number of active uniforms and attributes.
		int numberOfAttributes, numberOfUniforms;
		glGetProgramiv(m_ShaderID, GL_ACTIVE_ATTRIBUTES, &numberOfAttributes);
		glGetProgramiv(m_ShaderID, GL_ACTIVE_UNIFORMS, &numberOfUniforms);
		m_Uniforms.resize(numberOfUniforms);
		m_Attributes.resize(numberOfAttributes);

		//Iterate over all active attributes.
		char buffer[128];
		for (unsigned int i = 0; i < numberOfAttributes; i++)
		{
			GLenum glType;
			glGetActiveAttrib(m_ShaderID, i, sizeof(buffer), 0, &m_Attributes[i].m_AttributeSize, &glType, buffer);
			m_Attributes[i].m_AttributeName = std::string(buffer);
			m_Attributes[i].m_AttributeType = Shader_Type_Boolean; ///To be converted properly.

			m_Attributes[i].m_AttributeLocation = glGetAttribLocation(m_ShaderID, buffer); 
		}

		//Iterate over all active uniforms.
		for (unsigned int i = 0; i < numberOfUniforms; i++)
		{
			GLenum glType;
			glGetActiveUniform(m_ShaderID, i, sizeof(buffer), 0, &m_Uniforms[i].m_UniformSize, &glType, buffer);
			m_Uniforms[i].m_UniformName = std::string(buffer);
			m_Uniforms[i].m_UniformType = Shader_Type_Boolean; ///To be converted properly.

			m_Uniforms[i].m_UniformLocation = glGetUniformLocation(m_ShaderID, buffer);
		}
	}

	void Shader::UseShader()
	{
		glUseProgram(m_ShaderID);
	}

	bool Shader::HasUniform(const std::string& uniformName)
	{
		for (unsigned int i = 0; i < m_Uniforms.size(); i++)
		{
			if (m_Uniforms[i].m_UniformName == uniformName)
			{
				return true;
			}
		}
		return false;
	}

	void Shader::SetUniformFloat(std::string name, float value) 
	{
		int location = RetrieveUniformLocation(name);
		if (location >= 0)
		{
			glUniform1f(location, value);
		}
	}

	void Shader::SetUniformInteger(std::string name, int value) 
	{
		int location = RetrieveUniformLocation(name);
		if (location >= 0)
		{
			glUniform1i(location, value);
		}
	}

	void Shader::SetUniformBool(std::string name, bool value) 
	{
		int location = RetrieveUniformLocation(name);
		if (location >= 0)
		{
			glUniform1i(location, (int)value);
		}
	}

	void Shader::SetUniformVector2(std::string name, glm::vec2 value)
	{
		int location = RetrieveUniformLocation(name);
		if (location >= 0)
		{
			glUniform2fv(location, 1, &value[0]);
		}
	}

	void Shader::SetUniformVector3(std::string name, glm::vec3 value)
	{
		int location = RetrieveUniformLocation(name);
		if (location >= 0)
		{
			glUniform3fv(location, 1, &value[0]);
		}
	}

	void Shader::SetUniformMat4(std::string name, glm::mat4 value)
	{
		int location = RetrieveUniformLocation(name);
		if (location >= 0)
		{
			glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
		}
	}

	void Shader::SetUniformVectorArray(std::string name, int size, const std::vector<glm::vec3>& values)
	{
		unsigned int location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location >= 0)
		{
			glUniform3fv(location, size, (float*)(&values[0].x));
		}
	}

	int Shader::RetrieveUniformLocation(const std::string& uniformName)
	{
		for (unsigned int i = 0; i < m_Uniforms.size(); i++)
		{
			if (m_Uniforms[i].m_UniformName == uniformName)
			{
				return m_Uniforms[i].m_UniformLocation;
			}
		}
		return -1;
	}

	//====================================================================================================
	void Shader::DeleteShader()
	{
		glDeleteProgram(m_ShaderID);
	}

	void Shader::SetUniformVectorMat4(std::string identifier, std::vector<glm::mat4> value)
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

	Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		CreateShaders(vertexShaderPath, fragmentShaderPath);
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
}

