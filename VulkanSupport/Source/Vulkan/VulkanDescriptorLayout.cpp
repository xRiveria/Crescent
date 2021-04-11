#include "VulkanDescriptorLayout.h"
#include <array>
#include <stdexcept>
#include <iostream>

namespace Crescent
{
	VulkanDescriptorLayout::VulkanDescriptorLayout(VkDevice* logicalDevice) : m_LogicalDevice(logicalDevice)
	{
		CreateDescriptorSetLayout();
	}

	void VulkanDescriptorLayout::DestroyDescriptorLayoutInstance()
	{
		vkDestroyDescriptorSetLayout(*m_LogicalDevice, m_DescriptorSetLayout, nullptr);
	}

	void VulkanDescriptorLayout::CreateDescriptorSetLayout()
	{
		/*
			As we are able to pass arbitrary attributes to the vertex shader for each vertex, what about global variables? We're going to move on to 3D graphics and this
			requires a model-view-projection matrix. We include it as vertex data, but that's a waste of memory and it would require us to update the vertex buffer whenever
			the transformation changes. The transformation could easily change every single frame.

			The right way to tackle this in Vulkan is to use resource descriptors. A descriptor is a way for shaders to freely access resources like buffers and images.
			We're going to set up a buffer that contains the transformation matrices and have the vertex shader access them through a descriptor. The usage of such descriptors
			consists of three parts: Specifying a descriptor layout during pipeline creation, allocating a descriptor set from a descriptor pool and finally binding the set
			during rendering.

			The descriptor layout specifies the types of resources that are going to be accessed by the pipeline, just like a render pass specifies the types of attachments that
			will be accessed. A descriptor set specifies the actual buffer or image resources that will be bound to the descriptors, just like a framebuffer specifies the
			actual image views to bind to the render pass attachments. The descriptor set is then bound for the drawing commands just like the vertex buffers and framebuffer.

			There are many types of descriptors, but we will work for UBO (uniform buffer objects). Every binding needs to be first described through a VkDescriptorSetLayoutBinding struct.

			The first two fields specify the binding used in the shader and the type of descriptor it is, which is a uniform buffer object. It is possible for the shader variable to
			represent an array of uniform buffer objects, and descriptorCount specifies the amount of values in the array. This could be used to specify a transformation for each of the
			bones in a skeleton for skeletal animation, for example. Our MVP transformation is in a single uniform buffer object, so we're using a descriptorCount of 1.
		*/
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		/*
			We also need to specify in which shader stages the descriptor is going to be referenced. The stageFlags field can be a combination of VkShaderStageFlagBits values
			or the value VK_SHADER_STAGE_ALL_GRAPHICS. In our case, we're only referencing the descriptor from the vertex shader.
		*/
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		//The pImmutableSamplers field is only relevant for image sampling related descriptors, which we will look at later. You can leave this to its default value.
		layoutBinding.pImmutableSamplers = nullptr; //Optional

		/*
			We looked at UBO descriptors above, but we will now look at a new type of descriptor: combined image sampler. This descriptor makes it possible for shaders to access
			an image resource through a sampler object like the one we created for textures.

			We will now add another VkDescriptorSetLayoutBinding for a combined image sampler descriptor.
		*/
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		/*
			Make sure to set the stageFlags flag to indicate that we intend to use the combined image sampler descriptor in the fragment shader. That's where the color
			of the fragment is going to be determined. It is possible to use texture sampling in the vertex shader, for example, to dynamically deform a grid of vertices by a heightmap.
		*/
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { layoutBinding, samplerLayoutBinding };
		/*
			All of the descriptor bindings are combined into a single VkDescriptorSetLayoutBinding object. We can then create it using vkCreateDescriptorSetLayout. This function
			accepts a single VkDescriptorSetLayoutCreateInfo with the array of bindings. 
		*/
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(*m_LogicalDevice, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Descriptor Set Layout.\n");
		}
		else
		{
			std::cout << "Successfully created Descriptor Set Layout.\n";
		}
	}
}