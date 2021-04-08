#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

/*
	Mini I/O reading system meant to read shader files.
*/

static std::vector<char> ParseFile(const std::string& fileName)
{
	/*
		The "ate" flag will start reading at the end of the file, meaning that the output position starts at the end of the file.
		"Binary" indicates that we are reading the file as a binary file to avoid text transformations.
		The advantage of starting to read at the end of the file is that we can use the read position to determine the size of the file and allocate a buffer.
	*/
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file at: " + fileName);
	}

	//Retrieves the position of the current character in the input stream. Since its currently at the end due to the ate flag, hence the size of the buffer.
	size_t fileSize = (size_t)file.tellg(); 
	std::vector<char> buffer(fileSize);

	file.seekg(0); //Sets the position of the next character to be extracted from the input stream. We set it back to the start of the file.
	file.read(buffer.data(), fileSize); //Extract the amount of characters (in this case fileSize) and stores them in the array (in this case buffer.data()).
	
	file.close();

	/*
		Make sure that the shaders are loaded correctly by printing the size of the buffers and checking if they match the actual file size in bytes. Note that the code
		does not need to be null-terminated since its binary code and we will later be explicit about its size. 
	*/
	std::cout << "The size of " << fileName << " is " << buffer.size() << " bytes.\n";

	return buffer;
}