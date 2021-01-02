#include "CrescentPCH.h"
#include "ShaderLoader.h"
#include "../Utilities/StringID.h"
#include <string>

namespace Crescent
{
	Shader ShaderLoader::LoadShader(const std::string& shaderName, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, std::vector<std::string> defines)
	{
		std::ifstream vertexShaderFile, fragmentShaderFile;

		vertexShaderFile.open(vertexShaderFilePath);
		fragmentShaderFile.open(fragmentShaderFilePath);

		//If either one of the two files don't exist, return with an error message.
		if (!vertexShaderFile.is_open() || !fragmentShaderFile.is_open())
		{
			CrescentWarn("Shader failed to load at path " + vertexShaderFilePath + " and " + fragmentShaderFilePath);
			return Shader();
		}

		//Retrieve directory (for relative paths in shader includes).
		//std::string vertexShaderDirectory = vertexShaderFilePath.substr(0, vertexShaderFilePath.find_last_of("/\\"));
		//std::string fragmentShaderDirectory = fragmentShaderFilePath.substr(0, fragmentShaderFilePath.find_last_of("/\\"));

		std::string vertexShaderSource = ReadShader(vertexShaderFile, shaderName, vertexShaderFilePath);
		std::string vertexFragmentSource = ReadShader(fragmentShaderFile, shaderName, fragmentShaderFilePath);

		//Now build the shader with source code.
		Shader shader(shaderName, vertexShaderFilePath, fragmentShaderFilePath, defines);

		vertexShaderFile.close();
		fragmentShaderFile.close();

		return shader;
	}

	std::string ShaderLoader::ReadShader(std::ifstream& shaderFile, const std::string& shaderName, const std::string& shaderFilePath)
	{
		std::string directory = shaderFilePath.substr(0, shaderFilePath.find_last_of("/\\"));
		std::string source, line;
		while (std::getline(shaderFile, line))
		{
			//If we encounter an #include line, include another shader source.
			if (line.substr(0, 8) == "#include")
			{
				std::string includePath = directory + "/" + line.substr(9);
				std::ifstream includeFile(includePath);
				
				if (includeFile.is_open())
				{
					//We recursively read the shader file to support any shader include depth.
					source += ReadShader(includeFile, shaderName, includePath);
				}
				else
				{
					CrescentWarn("Shader: " + shaderName + ": include: " + includePath + " failed to open.");
				}
				includeFile.close();
			}
			else
			{
				source += line + "\n";
			}

			return source;
		}
	}
}