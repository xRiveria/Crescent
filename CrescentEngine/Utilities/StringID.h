#pragma once
#include <string>

/*
	Transforms any string into a fixed-size 32-bit integer for efficient identification purposes while keeping all the human-readable advantages of strings.
	Strings require more memory, dynamic memory allocation and are more expensive to compare than pure integers. Seeing as any game/graphics related engine
	has heavy requirements on those 3 mentioned points, deciding on a proper unique identification scheme is quite relevant.

	We create a hash function named StringID that does the string to integer conversion. Note than hashing is relatively expensive; its best o hash a string to get its unique identifier
	and then use the integer ID for all further computations.
*/

#define StringID(string) Custom_Simple_Hash(string)

inline unsigned int Custom_Simple_Hash(const std::string& string)
{
	unsigned int hash = 0;

	for (auto& iterator : string)
	{
		hash = 37 * hash + 17 * static_cast<char>(iterator);
	}

	return hash;
}

inline unsigned int Custom_Simple_Hash(const char* cString)
{
	std::string string(cString);

	return Custom_Simple_Hash(cString);
}