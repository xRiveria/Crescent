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

	std::string ShaderLoader::ReadShader(std::ifstream& file, const std::string& shaderName, std::string& filePath)
	{
		std::string directory = filePath.substr(0, filePath.find_last_of("/\\"));
		std::string source, line;

		while (std::getline(file, line))
		{
			//If we encounter any #include lines, its means we have another shader source that we wish to add to the current file.
			if (line.substr(0, 8) == "#include") //Because #include will always be 8 characters on a single line spanning from the start of the aforementioned line.
			{
				std::string includePath = directory + "/" + line.substr(9); //Grab the shader include's directory spanning from character position 9 to the end of the line.
				std::ifstream includeFile(includePath);

				CrescentLoad("Loading shader include for: " + shaderName);
				if (includeFile.is_open())
				{
					//We recursively read the shader file to support any shader include depth.
					source += ReadShader(includeFile, shaderName, includePath);
				}
				else
				{
					CrescentError("Shader include loading failed for: " + shaderName + " - " + includePath);
				}
				
				includeFile.close();
			}
			else
			{
				source += line + "\n";
			}
		}

		return source;
	}
}