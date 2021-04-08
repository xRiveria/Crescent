#include "VulkanShader.h"
#include "../Utitilies/FileSystem.h"

namespace Crescent
{
	VulkanShader::VulkanShader(const std::string& shaderFilePath, VkShaderStageFlagBits shaderStage, VkDevice* logicalDevice) : 
		m_ShaderFilePath(shaderFilePath), m_ShaderStage(shaderStage), m_LogicalDevice(logicalDevice)
	{
		CreateShaderModule();
	}

	void VulkanShader::DestroyShaderInstance()
	{
		vkDestroyShaderModule(*m_LogicalDevice, m_ShaderModule, nullptr);
	}

	VkPipelineShaderStageCreateInfo VulkanShader::RetrieveShaderStageInfo()
	{
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
		//The first step besides the obligatory sType member is telling Vulkan in which pipeline stage the shader is going to be used. There is an enum value for each of the programmable shader stages.
		shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCreateInfo.stage = m_ShaderStage;

		/*
			The next two members specify the shader module containing the code, and the function to invoke known as the entry point. That means that its possible to combine
			multiple fragment shaders into a single shader module, and use different entry points to differentiate between their behaviors. In that case, we will stick
			the standard "main". There is one more optional member, pSpecializationInfo which we won't be using here. It allows you to specify values for
			shader constants. You can use a single shader module where its behavior can be configured at pipeline creation by specifying different values for the
			constants used in it. This is more efficient than configuring the shader using variables at render time, because the compiler can do optimizations like eliminating
			if statements that depend on these values. If you do not have any constants like this, then you can set the member to nullptr which our struct initialization does automagically.
		*/
		shaderStageCreateInfo.module = m_ShaderModule;
		shaderStageCreateInfo.pName = "main";

		return shaderStageCreateInfo;
	}

	void VulkanShader::CreateShaderModule()
	{
		/*
			Shader modules are just a thin wrapper around the shader bytecode that we load from a file and the functions defined in it. The compilation and
			linking of the SPIR-V bytecode to machine code for execution by the GPU does not happen until the graphics pipeline is created. That means that we are
			allowed to destroy the shader modules as soon as the pipeline creation is finished.

			Unlike earlier APIs, shader code in Vulkan has to be specified in a bytecode format as opposed to human readable syntax like GLSL and HLSL. This
			bytecode format is called SPIR-V and is designed to be used with Vulkan and OpenCL. It is a format that can be used to write graphics and compute
			shaders. The advantage of using a bytecode format is that the compilers written by GPU vendors to turn shader code into native code are significantly
			less complex. The past has shown that with a human-readable syntax like GLSL, some GPU vendors were rather flexible with their intepretation of the standard.
			If you happen to write non-trivial shaders with a GPU from one of these vendors, then you would risk other vendor's drivers rejecting your code due to 
			syntax errors or worse, your shader running differently because of compiler bugs. With a straightforward bytecode format like SPIR-V, that will hopefully
			be avoided.

			Luckily, Khronos has released their own vendor-independant compiler that compiles GLSL to SPIR-V. This compiler is designed to verify that your shader
			code is fully standards compliant and produces one SPIR-V binary that you can ship with your p[rogram.

			Before we can pass our shader code to the pipeline, we have to first wrap it in a VkShaderModule object. This function will take a buffer with the bytecode
			as parameter and create a VkShaderModule from it.
		*/
		std::vector<char> shaderCode = ParseFile(m_ShaderFilePath);

		VkShaderModuleCreateInfo shaderModuleInfo{};
		shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleInfo.codeSize = shaderCode.size();
		/*
			The catch here is that the size of the bytecode is specified in bytes, but the bytecode pointer is a uint32_t pointer rather than a char pointer. Therefore,
			we will need to cast pointer with a reintepret cast. When you perform a cast like this, you also need to ensure that the data satisfies the alignment requirements
			of uint32_t. Lucky for us, the data is stored in a std::vector where the default allocator already ensures that the data satisfies the worst case alignment
			requirements.

			reinterpret_cast is used to convert one pointer to another pointer of any type, no matter whether the class is related to each other or not. It does not check
			if the pointer type and data pointed by the pointer is the same or not.
		*/
		shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		if (vkCreateShaderModule(*m_LogicalDevice, &shaderModuleInfo, nullptr, &m_ShaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Shader Module with path: " + m_ShaderFilePath);
		}
		else
		{
			std::cout << "Successfully created Shader Module with path: " + m_ShaderFilePath << "\n";
		}
	}
}