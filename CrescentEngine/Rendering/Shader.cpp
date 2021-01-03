#include "CrescentPCH.h"
#include "Shader.h"
#include "GL/glew.h"
#include <glm/gtc/type_ptr.hpp>

namespace Crescent
{
	Shader::Shader()
	{

	}

	Shader::Shader(const std::string& shaderName, std::string vertexShaderCode, std::string fragmentShaderCode, std::vector<std::string> defines)
	{
		LoadShader(shaderName, vertexShaderCode, fragmentShaderCode, defines);
	}

	void Shader::LoadShader(const std::string& shaderName, std::string vertexShaderCode, std::string fragmentShaderCode, std::vector<std::string> defines)
	{
		m_ShaderName = shaderName;
		
		//Compile both shaders and link them.
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		m_ShaderID = glCreateProgram();

		int shaderStatus;
		char shaderLog[1024];

		//If a list of define statements is specified, add these to the start of the shader source so that we can selectively compile different shaders based on the defines we set.
		if (defines.size() > 0)
		{
			std::vector<std::string> vertexShaderMergedCode;
			std::vector<std::string> fragmentShaderMergedCode;
			//We determine if the user supplied a #version directive at the top of the shader code, in which case we extract it and ass it before the list of define code.
			//The GLSL version specifier is only valid as the first line of the GLSL code, otherwise the GLSL version defaults to 1.1.
			std::string firstLine = vertexShaderCode.substr(0, vertexShaderCode.find("\n"));
			if (firstLine.find("#version") != std::string::npos)
			{
				//Strip shader code of the first line and add it the list of shader code strings.
				vertexShaderCode = vertexShaderCode.substr(vertexShaderCode.find("\n") + 1, vertexShaderCode.length() - 1);
				vertexShaderMergedCode.push_back(firstLine + "\n");
			}
			firstLine = fragmentShaderCode.substr(0, fragmentShaderCode.find("\n"));
			if (firstLine.find("#version") != std::string::npos)
			{
				//Strip shader code of the first line and add it to the list of shader code strings.
				fragmentShaderCode = fragmentShaderCode.substr(fragmentShaderCode.find("\n") + 1, fragmentShaderCode.length() - 1);
				fragmentShaderMergedCode.push_back(firstLine + "\n");
			}
			//Then, add define statements to the shader string list.
			for (unsigned int i = 0; i < defines.size(); ++i)
			{
				std::string define = "#define " + defines[i] + "\n";
				vertexShaderMergedCode.push_back(define);
				fragmentShaderMergedCode.push_back(define);
			}
			//Note that we manually build an array of C styled strings as glShaderSource doesn't expect it in any other format.
			//All strings are null-terminated so we pass Null as glShaderSource's final argument.
			const char** vertexShaderStringsC = new const char*[vertexShaderMergedCode.size()]; //A pointer to a collection of Cstrings. 
			const char** fragmentShaderStringsC = new const char*[fragmentShaderMergedCode.size()];
			for (unsigned int i = 0; i < vertexShaderMergedCode.size(); ++i)
			{
				vertexShaderStringsC[i] = vertexShaderMergedCode[i].c_str();
			}
			for (unsigned int i = 0; i < fragmentShaderMergedCode.size(); ++i)
			{
				fragmentShaderStringsC[i] = fragmentShaderMergedCode[i].c_str();
			}
			glShaderSource(vertexShader, vertexShaderMergedCode.size(), vertexShaderStringsC, nullptr);
			glShaderSource(fragmentShader, fragmentShaderMergedCode.size(), fragmentShaderStringsC, nullptr);
			delete[] vertexShaderStringsC;
			delete[] fragmentShaderStringsC;
		}
		else
		{
			const char* vertexShaderCodeC = vertexShaderCode.c_str();
			const char* fragmentShaderCodeC = fragmentShaderCode.c_str();
			glShaderSource(vertexShader, 1, &vertexShaderCodeC, nullptr);
			glShaderSource(fragmentShader, 1, &fragmentShaderCodeC, nullptr);
		}
		glCompileShader(vertexShader);
		glCompileShader(fragmentShader);

		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderStatus);
		if (!shaderStatus)
		{
			glGetShaderInfoLog(vertexShader, 1024, nullptr, shaderLog);
			CrescentWarn("Vertex Shader Compilation Error At: " + shaderName + "!\n" + std::string(shaderLog));
		}

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderStatus);
		if (!shaderStatus)
		{
			glGetShaderInfoLog(fragmentShader, 1024, nullptr, shaderLog);
			CrescentWarn("Fragment Shader Compilation Error At: " + shaderName + "!\n" + std::string(shaderLog));
		}
		glAttachShader(m_ShaderID, vertexShader);
		glAttachShader(m_ShaderID, fragmentShader);
		glLinkProgram(m_ShaderID);

		glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &shaderStatus);
		if (!shaderStatus)
		{
			glGetProgramInfoLog(m_ShaderID, 1024, nullptr, shaderLog);
			CrescentWarn("Shader Program Linking Error At: " + shaderName + "!\n" + std::string(shaderLog));
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		//Query the number of active uniforms and attributes.
		int numberOfAttributes, numberOfUniforms;
		glGetProgramiv(m_ShaderID, GL_ACTIVE_UNIFORMS, &numberOfUniforms);
		glGetProgramiv(m_ShaderID, GL_ACTIVE_ATTRIBUTES, &numberOfAttributes);
		m_Uniforms.resize(numberOfUniforms);
		m_Attributes.resize(numberOfAttributes);

		//Iterate over all active attributes.
		char buffer[128];
		for (unsigned int i = 0; i < numberOfAttributes; ++i)
		{
			GLenum glType;
			glGetActiveAttrib(m_ShaderID, i, sizeof(buffer), 0, &m_Attributes[i].m_VertexAttributeSize, &glType, buffer);
			m_Attributes[i].m_VertexAttributeName = std::string(buffer);
			m_Attributes[i].m_VertexAttributeDataType = Shader_Type_Bool;

			m_Attributes[i].m_VertexAttributeLocation = glGetAttribLocation(m_ShaderID, buffer);
		}

		//Iterator over all active uniforms.
		for (unsigned int i = 0; i < numberOfUniforms; ++i)
		{
			GLenum glType;
			glGetActiveUniform(m_ShaderID, i, sizeof(buffer), 0, &m_Uniforms[i].m_UniformSize, &glType, buffer);
			m_Uniforms[i].m_UniformName = std::string(buffer);
			m_Uniforms[i].m_UniformDataType = Shader_Type_Bool;

			m_Uniforms[i].m_UniformLocation = glGetUniformLocation(m_ShaderID, buffer);
		}
	}

	void Shader::UseShader()
	{
		glUseProgram(m_ShaderID);
	}

	bool Shader::HasUniform(const std::string& name)
	{
		for (unsigned int i = 0; i < m_Uniforms.size(); ++i)
		{
			if (m_Uniforms[i].m_UniformName == name)
			{
				return true;
			}
		}
		return false;
	}

	void Shader::SetUniformFloat(const std::string& name, float value) 
	{
		int location = RetrieveUniformLocation(name);

		if (location >= 0)
		{
			glUniform1f(location, value);
		}
	}

	void Shader::SetUniformInteger(const std::string& name, int value) 
	{
		int location = RetrieveUniformLocation(name);

		if (location >= 0)
		{
			glUniform1i(location, value);
		}
	}

	void Shader::SetUniformBool(const std::string& name, bool value) 
	{
		int location = RetrieveUniformLocation(name);

		if (location >= 0)
		{
			glUniform1i(location, (int)value);
		}
	}

	void Shader::SetUniformVector3(const std::string& name, const glm::vec3& value)
	{
		int location = RetrieveUniformLocation(name);

		if (location >= 0)
		{
			glUniform3fv(location, 1, glm::value_ptr(value));
		}
	}

	void Shader::SetUniformMat4(const std::string& name, const glm::mat4& value)
	{
		int location = RetrieveUniformLocation(name);

		if (location >= 0)
		{
			glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
		}
	}

	void Shader::SetUniformVectorMat4(const std::string& name, const std::vector<glm::mat4>& value)
	{
		int location = RetrieveUniformLocation(name);

		if (location >= 0)
		{
			glUniformMatrix4fv(location, value.size(), GL_FALSE, value_ptr(value[0]));
		}
	}

	int Shader::RetrieveUniformLocation(const std::string& name)
	{
		//Read from uniform/attribute array as originally obtained from OpenGL.
		for (unsigned int i = 0; i < m_Uniforms.size(); ++i)
		{
			if (m_Uniforms[i].m_UniformName == name)
			{
				return m_Uniforms[i].m_UniformLocation;
			}
		}
		return -1;
	}


	//====

	void Shader::UnbindShader()
	{
		glUseProgram(0);
	}

	void Shader::DeleteShader()
	{
		glDeleteProgram(m_ShaderID);
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

	Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		CreateShaders(vertexShaderPath, fragmentShaderPath);
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

