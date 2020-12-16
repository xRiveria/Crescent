#pragma once
#include <memory>
#include <string>
#include <map>

namespace CrescentEngine
{
	class BoneMapper
	{
	public:
		BoneMapper();
		uint32_t Name(const std::string& name);
		void Clear();

		uint32_t RetrieveTotalBones() const { return m_TotalBones; }
		std::map<std::string, uint32_t>& RetrieveBoneLibrary() { return m_BoneLibrary; }

	private:
		std::map<std::string, uint32_t> m_BoneLibrary;
		uint32_t m_TotalBones;
	};
}
