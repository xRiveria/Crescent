#pragma once

#include <string>
#include <fstream>
#include "../Shading/Shader.h"

namespace Crescent
{
	/*
		A static helper class that does the relevant file IO and custom shader pre-processing to load and parse shader code.
	*/

	class ShaderLoader
	{
	public:
		static Shader LoadShader(const std::string& shaderName, std::string vertexShaderPath, std::string fragmentShaderPath);

	private:
		static std::string ReadShader(std::ifstream& file, const std::string& fileName, std::string& filePath);
	};
}