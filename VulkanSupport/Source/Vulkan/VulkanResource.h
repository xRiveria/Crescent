#pragma once
#include "VulkanUtilities.h"

namespace Crescent
{
	class VulkanResource
	{
	public:
		VulkanResource(const std::string& modelFilePath);

		//std::vector<Vertex>* RetrieveVertices() { return &m_Vertices; }
		//std::vector<uint32_t>* RetrieveIndices() { return &m_Indices; }
			
		void LoadModel();

	public:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices; //We are using uint32_t as there will be more vertices than 65535.

	private:
		std::string m_FilePath;
	};
}