#include "CrescentPCH.h"
#include "ShaderLoader.h"
#include "../Shading/Shader.h"

namespace Crescent
{
	Shader ShaderLoader::LoadShader(const std::string& shaderName, std::string vertexShaderPath, std::string fragmentShaderPath)
	{
		std::ifstream vertexShaderFile, fragmentShaderFile;
		vertexShaderFile.open(vertexShaderPath);
		fragmentShaderFile.open(fragmentShaderPath);

		//If either of the two sides don't exist, we return with an error message.
		if (!vertexShaderFile.is_open() || !fragmentShaderFile.is_open())
		{
			CrescentError("Shader failed to load at path: " + vertexShaderPath + " and " + fragmentShaderPath);
			return Shader();
		}

		//Retrieve directory (for relative paths in shader includes).
		std::string vertexFileDirectory = vertexShaderPath.substr(0, vertexShaderPath.find_last_of("/\\"));
		std::string fragmentFileDirectory = fragmentShaderPath.substr(0, fragmentShaderPath.find_last_of("/\\"));

		std::string vertexSource = ReadShader(vertexShaderFile, shaderName, vertexShaderPath);
		std::string fragmentSource = ReadShader(fragmentShaderFile, shaderName, fragmentShaderPath);

		//Now, we build the shader with the source code.
		Shader shader(shaderName, vertexSource, fragmentSource);

		vertexShaderFile.close();
		fragmentShaderFile.close();

		return shader;
	}

	std::string ShaderLoader::ReadShader(std::ifstream& file, const std::string& fileName, std::string& filePath)
	{
		std::string directory = filePath.substr(0, filePath.find_last_of("/\\"));
		std::string source, line;

		while (std::getline(file, line))
		{
			source += line + "\n";
		}

		return source;
	}
}