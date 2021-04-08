#pragma once
#include <vulkan/vulkan.h>
#include <string>

namespace Crescent
{
	class VulkanShader
	{
	public:
		VulkanShader(const std::string& shaderFilePath, VkShaderStageFlagBits shaderStage, VkDevice* logicalDevice);
		void DestroyShaderInstance();

		VkShaderModule* RetrieveShaderModule() { return &m_ShaderModule; }
		VkPipelineShaderStageCreateInfo RetrieveShaderStageInfo();

	private:
		void CreateShaderModule();

	private:
		std::string m_ShaderFilePath;
		VkShaderModule m_ShaderModule;
		VkShaderStageFlagBits m_ShaderStage;

		VkDevice* m_LogicalDevice;
	};
}