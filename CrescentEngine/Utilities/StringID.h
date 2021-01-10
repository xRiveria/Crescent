#pragma once

#define SID(string) Custom_Simple_Hash(string)

inline unsigned int Custom_Simple_Hash(const std::string& string)
{
	unsigned int hash = 0;

	for (auto& iterator : string)
	{
		hash = 37 * hash + 17 * static_cast<char>(iterator);
	}

	return hash;
}