#include "VulkanDescriptorPool.h"
#include <array>
#include <stdexcept>
#include <iostream>

namespace Crescent
{
	VulkanDescriptorPool::VulkanDescriptorPool(std::vector<std::shared_ptr<VulkanTexture>>* swapchainImages, VkDevice* logicalDevice) : m_SwapchainImages(swapchainImages), m_LogicalDevice(logicalDevice)
	{
		CreateDescriptorPool();
	}

	void VulkanDescriptorPool::CreateDescriptorPool()
	{
		/*
			We will create a descriptor set for each VkBuffer resource to bind it to the uniform buffer descriptor. Now, descriptor sets cannot be created directly but rather must 
			be allocated from a pool like command buffers. The equivalant for descriptor sets is unsurprisingly called a descriptor pool. First, we will need to describe which 
			descriptor types our descriptor sets are going to contain and how many of them.
		*/
		std::array<VkDescriptorPoolSize, 2> poolSizeInfo{};
		poolSizeInfo[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizeInfo[0].descriptorCount = static_cast<uint32_t>(m_SwapchainImages->size());

		poolSizeInfo[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizeInfo[1].descriptorCount = static_cast<uint32_t>(m_SwapchainImages->size());

		/*
			Inadequete descriptor pools are a good example of a problem that the validation layers will not catch. As of Vulkan 1.1, vkAllocateDescriptorSets may fail with 
			the error code VK_ERROR_POOL_OUT_OF_MEMORY if the pool is not sufficiently large, but the driver may also try to solve the problem internally. This means that sometimes 
			(depending on hardware, pool size and allocation size), the driver will let us get away with allocation that exeeds that limits of our descriptor pool.

			Other times, vkAllocateDescriptorSets will fail and return VK_ERROR_POOL_OUT_OF_MEMORY. This can be particularly frustrating if the allocation succeeds on some machines, 
			but fails on others.

			Since Vulkan shifts the responsibility for the allocation to the driver, it is no longer a strict requirement to only allocate as many descriptors of a certain type 
			(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, etc) as specified by the corresponding descriptorCount members for the creation of the descriptor pool. However, it remains 
			best practice to do so and in the future, VK_LAYER_KHRONOS_validation will warn about this type of problem if you enable Best Practice Validation.

			We will allocate one of these descriptors every frame. This pool size struture is referenced by the main VkDescriptorPoolCreateInfo.
		*/
		VkDescriptorPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizeInfo.size());
		poolCreateInfo.pPoolSizes = poolSizeInfo.data();
		//Aside from the maximum number of individual descriptors that are avaliable, we also need to specify the maximum number of descriptor sets that may be allocated.
		poolCreateInfo.maxSets = static_cast<uint32_t>(m_SwapchainImages->size());
		//The structure has an optional flag simmilar to command pools that determines if individual descriptor sets can be freed or not: VK_DESCRIPTOR_CREATE_FREE_DESCRIPTOR_SET_BIT.
		//We're not going to touch the descriptor set after creating it, so we don't need this flag.
		poolCreateInfo.flags = 0;

		if (vkCreateDescriptorPool(*m_LogicalDevice, &poolCreateInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Descriptor Pool.\n");
		}
		else
		{
			std::cout << "Successfully created Descriptor Pool.\n";
		}
	}
}