#pragma once
#include <string>
#include <fstream>
#include <vector>
#include "../Rendering/Shader.h"

namespace CrescentEngine
{
	/*
		A static helper class that does the relevant file IO, custom shader pre-processing to load and parse shader code.
	*/

	class ShaderLoader
	{
	public:
		static Shader LoadShader(const std::string& shaderName, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, std::vector<std::string> defines = std::vector<std::string>());

	private:
		static std::string ReadShader(std::ifstream& shaderFile, const std::string& shaderName, const std::string& shaderFilePath);
	};
}