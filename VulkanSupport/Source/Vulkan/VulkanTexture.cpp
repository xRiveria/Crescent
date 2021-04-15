#include "VulkanTexture.h"
#include <stdexcept>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "../../Vendor/stb_image/stb_image.h"
#include "VulkanUtilities.h"

namespace Crescent
{
	VulkanTexture::VulkanTexture(const std::string& filePath, VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkFormat imageFormat, const VkImageAspectFlags& imageAspectFlags, VkCommandPool* commandPool, VkQueue* queue) 
		: m_LogicalDevice(logicalDevice), m_PhysicalDevice(physicalDevice), m_TextureFormat(imageFormat), m_TextureTypeFlag(imageAspectFlags), m_CommandPool(commandPool), m_Queue(queue)
	{
		/*
			The stbi_load function takes the file path and number of channels to load as arguments. The STBI_rgb_alpha value forces the the image to be loaded with an alpha channel,
			even if it doesn't have one, which is nice for consistency with other textures in the future. The middle three parameters are output for width, height and actual
			number of channels in the image. The pointer that is returned is the first element in an array of pixel values. The pixels are load out row by row with 4 bytes
			per pixel in the case of STBI_rgb_alpha for a total of textureWidth * textureHeight * 4 values.
		*/
		int textureWidth, textureHeight, textureChannels;
		stbi_uc* pixels = stbi_load(filePath.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = textureWidth * textureHeight * 4; //VkDeviceSize represents device memory size and offset values. It is internally a uint64_t.

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image with path: " + filePath);
		}

		/*
			We need to calculate the number of mipmap levels avaliable from the dimensions of the image. This calculates the number of levels in the mip chain. The max function 
			selects the largest dimension. The log2 function calculates how many times that dimension can be divided by 2. The floor function handles cases where the 
			largest dimension is not a power of 2. 1 is added so that the original image has a mip level.
		*/
		m_MipmapLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;

		//We're going to create a buffer in host visible memory so that we can use vkMapMemory and copy the pixels loaded from our image library to it. 
		//This buffer should not only be in host visible memory but also be able to be used as a transfer source so we can copy it to an image eventually.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, *m_LogicalDevice, *m_PhysicalDevice);

		//We can then directly copy the pixel values that we got from the image loading library to the buffer.
		void* dataBuffer;
		vkMapMemory(*m_LogicalDevice, stagingBufferMemory, 0, imageSize, 0, &dataBuffer); //After a successful call to vkMapMemory, the memory object is considered to be currently host mapped.
		memcpy(dataBuffer, pixels, static_cast<uint32_t>(imageSize));
		vkUnmapMemory(*m_LogicalDevice, stagingBufferMemory);

		//Clean up the original pixel array once mapped.
		stbi_image_free(pixels);

		//Create our Vulkan image object. This will be a 4 component, 32-bit unsigned normalized format that has 8 bits per component.
		CreateTexture(textureWidth, textureHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Texture, m_TextureMemory);
		CreateTextureView();
		CreateTextureSampler();

		/*
			Now, we are going to copy the staging buffer in host visible memory to the texture image. This involves 2 steps:

				1) Transitioning the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
				2) Executing the buffer-to-image operation.

			The image was created with the VK_IMAGE_UNDEFINED layout, so that layout should be specified as the old layout when transitioning the texture image. Remember that
			we can do this because we don't care about its contents prior to the copy operation.
		*/
		TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, *m_CommandPool, *m_Queue);

		CopyBufferToImage(stagingBuffer, static_cast<uint32_t>(textureWidth), static_cast<int32_t>(textureHeight));

		//To be able to start sampling from the image texture in the shader, we need one last transition to prepare it for shader access. We will do this from GenerateMipmaps.
		//TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, *m_CommandPool, *m_Queue);
		GenerateMipmaps(m_Texture, VK_FORMAT_R8G8B8A8_SRGB, textureWidth, textureHeight, m_MipmapLevels);

		vkDestroyBuffer(*m_LogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*m_LogicalDevice, stagingBufferMemory, nullptr);
	}

	VulkanTexture::VulkanTexture(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, const VkImage& image, const VkFormat& imageFormat, const VkImageAspectFlags& imageAspectFlags)
		: m_LogicalDevice(logicalDevice), m_PhysicalDevice(physicalDevice), m_Texture(image), m_TextureFormat(imageFormat), m_TextureTypeFlag(imageAspectFlags)
	{
		CreateTextureView();
		CreateTextureSampler();
	}

	VulkanTexture::VulkanTexture(const int& textureWidth, const int& textureHeight, VkFormat format, const VkImageAspectFlags& imageAspectFlags, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice)
		: m_TextureFormat(format), m_TextureTypeFlag(imageAspectFlags), m_LogicalDevice(logicalDevice), m_PhysicalDevice(physicalDevice)
	{
		CreateTexture(textureWidth, textureHeight, m_TextureFormat, tiling, usage, properties, m_Texture, m_TextureMemory);
		CreateTextureView();
		CreateTextureSampler();
	}

	void VulkanTexture::DeleteTextureViewInstance()
	{
		vkDestroyImageView(*m_LogicalDevice, m_TextureView, nullptr);
	}

	void VulkanTexture::DeleteTextureImageInstance()
	{
		vkDestroyImage(*m_LogicalDevice, m_Texture, nullptr);
	}

	void VulkanTexture::DeleteTextureMemoryInstance()
	{
		vkFreeMemory(*m_LogicalDevice, m_TextureMemory, nullptr);
	}

	void VulkanTexture::DeleteTextureSamplerInstance()
	{
		vkDestroySampler(*m_LogicalDevice, m_TextureSampler, nullptr);
	}

	void VulkanTexture::DestroyAllTextureInstances()
	{
		vkDestroyImageView(*m_LogicalDevice, m_TextureView, nullptr);
		vkDestroyImage(*m_LogicalDevice, m_Texture, nullptr);
		vkFreeMemory(*m_LogicalDevice, m_TextureMemory, nullptr);
		vkDestroySampler(*m_LogicalDevice, m_TextureSampler, nullptr);
	}

	//Represents an abstract texture creation function. The width, height, format, tiling mode, usage and memory properties parameters are different as they vary between images.
	void VulkanTexture::CreateTexture(const int& textureWidth, const int& textureHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		/*
			Although we could set up the shader to access the pixel values directly in the buffer, its better to use image objects in Vulkan for this purpose. For one, image objects
			will make it easier and faster to retrieve colors by allowing us to use 2D coordinates. Pixels within an image object are known as texels.

			The image type, specified in the imageType field, tells Vulkan with what kind of coordinate system the texels in the image are going to be addressed. It is
			possible to create 1D, 2D or 3D images. One dimensional images can be used to store an array or gradient, two dimensional images are mainly used for textures,
			and 3 dimensional images can be used to store voxel volumes for example. The extent field specifies the dimensions of the image, basically how many texels there are
			on each axis. That's why the depth must be 1 instead of 0. Our texture will not be an array and we won't be using mipmapping for now.
		*/
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(textureWidth);
		imageInfo.extent.height = static_cast<uint32_t>(textureHeight);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = m_MipmapLevels; //A progressively lower resolution representation of the previous one for optimization.
		imageInfo.arrayLayers = 1;
		//Vulkan supports many possible image formats, but we should use the same format for the texels as the pixels in the buffer. Otherwise, the copy operation will fail.
		imageInfo.format = format;

		/*
			The tiling field can have one of two values:
			
			- VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major like our pixels array.
			- VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined order for optimal access.

			Unlike the layout of an image, the tiling mode cannot be changed at a later time. If you want to be able to directly access texels in the memory of an image,
			then you must use the VK_IMAGE_TILING_LINEAR. We will be using a staging buffer instead of a staging image, so this won't be necessary. We will be using
			VK_IMAGE_TILING_OPTIMAL for efficient access from the shader.
		*/
		imageInfo.tiling = tiling;
		/*
			There are only two possible values for the initialLayout of an image:

			- VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
			- VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.

			There are few situations where it is necessary for the texels to be preserved during the first transition. One example, however, would be if you wanted to use an
			image as a staging image in combinationm with the VK_IMAGE_TILING_LINEAR layout. In that case, you would want to upload the texel data to it and then
			transition the image to be a transfer source without losing the data. In our case, however, we're going to first transition the image to be a transfer
			destination and copy the texel data to it from a buffer object, so we don't need this property anymore and can safely use VK_IMAGE_LAYOUT_UNDEFINED.
		*/
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		/*
			The usage field has the same semantics as the one during buffer creation. The image is going to be used as a destination for the buffer copy, so it should be
			setup as a transfer destination. We also want to be able to access the image from the shader to color our mesh, so the usage should include VK_IMAGE_USAGE_SAMPLED_BIT.
		*/
		imageInfo.usage = usage;
		//The image will only be used by one queue family: the one that supports graphics (and therefore also) transfer operations.
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		/*
			The samples flag is related to multisampling. This is only relevant for images that will be used as attachments, so stick to one sample. There are some
			optional flags for images that are related to sparse images. Sparse images are images where only certain regions are backed by memory. If you were
			using a 3D texture for a voxel terrain, for example, we could use this to avoid allocating memory to store large amounts of "air" values. We won't be using it,
			so lets leave it at its default value of 0.
		*/
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0; //Optional
		/*
			The image is created using vkCreateImage, which doesn't have any particularly noteworthy parameters. It is possible that the VK_FORMAT_R8G8B8A8_SRGB format'
			is not supported by the graphics hardware. Thus, we should have a list of acceptable alternatives and go with the best one that is supported. However,
			support for this particular format is so widespread that we will skip this step. Using different formats would also require annoying conversions. We will get
			back to this when doing up depth-buffers.
		*/
		if (vkCreateImage(*m_LogicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image.");
		}
		else
		{
			std::cout << "Successfully create Image.\n";
		}
		/*
			Allocating memory for an image works exactly in the same way as allocating memory for a buffer. Use vkGetImageMemoryRequirements instead of vkGetBufferMemoryRequirements,
			and use vkBindImageMemory instead of vkBindBufferMemory.
		*/
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(*m_LogicalDevice, image, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties, *m_PhysicalDevice);

		if (vkAllocateMemory(*m_LogicalDevice, &allocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory.");
		}
		
		vkBindImageMemory(*m_LogicalDevice, image, imageMemory, 0);
	}

	void VulkanTexture::CreateTextureView()
	{
		/*
			An image view is literally a view into an image and is required to use an VkImage. It describes how to access the image and which part of the image to access.
			For example, should it be treated as a 2D depth texture without any mipmapping levels?
		*/
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = m_Texture;
		//The viewType and format fields specifies how the image data should be interpreted. The viewType parameter allows you to treat images as 1D, 2D or 3D textures and cubemaps.
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = m_TextureFormat;
		//The components fields allow you to swizzle the color channels around. 
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		/*
			The subresourceRange fields describe what the image's purpose is and which part of the image should be accessed. Our images will be used as color targets
			without any mipmapping levels or multiple layers, for example.
		*/
		imageViewInfo.subresourceRange.aspectMask = m_TextureTypeFlag;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = m_MipmapLevels; //Our mipmap level.
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		//If this was a steoreographic 3D application, we would create a swapchain with multiple layers. We can then create multiple image views for each image representing
		//the views for the left and right eyes by accessing different layers.
		imageViewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(*m_LogicalDevice, &imageViewInfo, nullptr, &m_TextureView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Image View Texture.");
		}
		else
		{
			std::cout << "Successfully created Image View.\n";
		}
	}

	void VulkanTexture::CreateTextureSampler()
	{
		/*
			It is possible for shaders to read texels directly from images, but it is not very common when they are used as textures. Textures are usually accessed through 
			samplers, which will apply filtering and transformations to compute the final color that is retrieved. These filters are helpful to deal with problems like oversampling.
			Consider a texture that is mapped to geometry with more fragments than texels. If you simply took the closest texel for the texture coordinate in each fragment, you 
			would get quite a Minecraft styled pixelized image.

			However, if you combine the 4 closest texels through linear interpolation, then you would get a smoother result. Of course, your application may have art style requirements 
			that fit the pixelized style more, the cleaner one is more preferred in conventional graphics application.

			Undersampling is the opposite problem, where you have more texels than fragments. This will lead to artifacts when sampling high frequently patterns like a checkerboard 
			texture at a sharp angle. Without anisotropic filtering, the texture turns into a blurry mess in the distance. Anisotropic filtering, however, is applied automatically 
			by a sampler.

			Aside from these filters, a sampler can also take care of transformations. It determines what happens when you try to read texels outside the image through its addressing
			mode: Repeat, Mirrored Repeat, Clamp to Edge and Clamp to Border.

			Samplers are configured through a VkSamplerCreateInfo structure, which specifies all filters and transformations that it should apply.
		*/
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		/*
			The magFilter and minFilter field specify how to interpolate texels that are magnified or minified. Magnification concerns the oversampling problems described above 
			and minification concerns undersampling. The choices are VK_FILTER_NEAREST and VK_FILTER_LINEAR, corresponding to the modes demonstrated in the images above.
		*/
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		/*
			The addressing mode can be specifed per axis using the addressMode fields. The avaliable values are listeds below. Note that the axes are called U, V and W instead 
			of X, Y and Z. This is a convention for texture space coordinates:

			- VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions.
			- VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts the coordinates to mirror the image when going beyond the dimensions.
			- VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color of the edge closest to the coordinate beyond the image dimensions.
			- VK_SAMPLER_ADDRESS_MIRROR_CLAMP_TO_EDGE: Like clamp to edge, but instead uses the edge opposite to the closest edge.
			- VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Return a solid color when sampling beyond the dimensions of the image.

			It doesn't really really matter which addressing mode we use here, because we're not going to sample outside the image for now. However, the repeat mode 
			is probably the most common mode, because it can be used to tile textures like floors and walls.
		*/
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		/*
			These two fields specify if anistropic filtering should be used. There is no reason not to use this unless peformance is a concern. The maxAnisotropy field 
			limits the amount of texels samples that can be used to calculate the final color. A lower value results in better performance, but lower quality results. To figure
			out which values we can use, we need to retrieve the properties of the physical device like so:
		*/
		samplerInfo.anisotropyEnable = VK_TRUE; //We can enforce the avaliability of anistropic filtering, its also possible to simply not use it by conditionally setting anistropyEnable = VK_FALSE.
		samplerInfo.maxAnisotropy = 5;

		VkPhysicalDeviceProperties physicalDeviceProperties{};
		vkGetPhysicalDeviceProperties(*m_PhysicalDevice, &physicalDeviceProperties);
		/*
			If you look at the documentation for the VkPhysicalDeviceProperties struct, you will see that it contains a VkPhysicalDeviceLimits member named limits. This 
			struct in turn has a member called maxSamplerAnistropy and this is the maximum value we can sample for maxAnistropy. If we want to go for maximum quality, we can 
			use that value directly.

			You can either query the properties at the beginning of your program and pass them around to the functions that need them, or query them here itself.
		*/
		samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
		/*
			The borderColor field specifies which color is returned when sampling beyond the image with clamp to border addressing mode. It is possible to return black, 
			white or transparent in either float or int formats. You cannot specify an arbitrary colors.
		*/
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		/*
			The unnormalizedCoordinates field specifies which coordinate system you want to use to address texels in an image. If this field is VK_TRUE, then you can 
			simply use coordinates within the [0, textureWidth] and [0, textureHeight] range. If it is VK_FALSE, then the texels are addressed using the [0, 1] range on 
			all axes. Real-world applications almost always use normalized coordinates, because then its possible to use textures of varying resolutions with the exact same 
			coordinates.
		*/
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		/*
			If a comparison function is enabled, then texels will first be compared to a value, and the result of that comparison is used in filtering operations. This is mainly 
			used for PCF (percentage closer filtering) on shadow maps. We will look at this in a future chapter.
		*/
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		/*
			While VkImage holds the mipmap data, VkSampler controls how that data is read while rendering. Vulkan allows us to specify minLod, maxLod, mipLodBias and mipmapMode.
			When a texture is sampled, the sampler selects a mip level accordingly. If samplerInfo.mipmapMode is VK_SAMPLER_MIPMAP_MODE_NEAREST, lod selects the mip level to sample
			from. If the mipmap mode is VK_SAMPLER_MIPMAP_MODE_LINEAR, lod is used to select two mip levels to be sampled. Those levels are sampled and the results are linearly 
			blended. 

			If the object is close to the camera, magFilter is used as the filter. If the object is further from the camera, minFilter is used. Normally, lod is non-negative, and
			is only 0 when close to the camera. mipLodBias lets us force Vulkan to use lower lod and level that it would normally use.

			To allow for the full range of mip levels to be used, we set minLod to 0.0f, and maxLod to the number of mip levels avaliable. We have no reason to change the lod value, 
			so we set the mipLodBias to 0.0f.
		*/
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; 
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(m_MipmapLevels);

		/*
			The functioning of the sampler is now fully defined. Add a class member to hold the handle of the sampler object and create the sampler with vkCreateSampler. Note 
			that the sampler does not reference a VkImage anywhere. The sampler is a distinct object that provides an interface to extract colors from a texture. It can 
			be applied to any image you want, whether it be 1D, 2D or 3D. This is different from many older APIs, which combines texture images and filtering into a single state.
		*/
		if (vkCreateSampler(*m_LogicalDevice, &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Texture Sampler.\n");
		}
		else
		{
			std::cout << "Successfully create Texture Sampler.\n";
		}
	}

	void VulkanTexture::CopyBufferToImage(VkBuffer buffer, uint32_t imageWidth, uint32_t imageHeight)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(*m_LogicalDevice, *m_CommandPool);
		//Just like with buffer copying, you need to specify which part of the buffer is going to be copied to which part of the image, done through VkBufferImageCopy structs.
		VkBufferImageCopy regionInfo{};
		regionInfo.bufferOffset = 0; //Byte offset in the buffer at which the pixel values start.
		/*
			Specifies how the pixels are laid out in memory. For example, you could have some padding bytes between rows of the image.
			Specifying 0 for both indicates that the pixels are simply tightly packed like they are in our case.
		*/
		regionInfo.bufferRowLength = 0;
		regionInfo.bufferImageHeight = 0;

		//The imageSubresource, imageOffset and imageExtent fields indicate to which part of the image we want to copy our pixels. In this case, we are copying to the entire image.
		regionInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		regionInfo.imageSubresource.mipLevel = 0;
		regionInfo.imageSubresource.baseArrayLayer = 0;
		regionInfo.imageSubresource.layerCount = 1;

		regionInfo.imageOffset = { 0, 0, 0 };
		regionInfo.imageExtent = { imageWidth, imageHeight, 1 }; //XYZ

		/*
			Buffer top image copy operations are enqueued using the vkCmdCopyBufferToImage function. The fourth parameter indicates which layout the image is currently using. We are 
			assuming here that the image has already been transitioned to the layout that is optimal for copying pixels to. Right now, we're only copying one chunk of pixels to the 
			whole image, but its possible to specify an array of VkBufferImageCopy to perform many different copies from this buffer to the image in one operation.
		*/
		vkCmdCopyBufferToImage(commandBuffer, buffer, m_Texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &regionInfo);

		EndSingleTimeCommands(commandBuffer, *m_CommandPool, *m_LogicalDevice, *m_Queue);
	}

	void VulkanTexture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, const VkCommandPool& commandPool, const VkQueue& queue)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(*m_LogicalDevice, commandPool);

		/*
			One of the most common ways to perform layout transitions is using an image memory barrier. A pipeline barrier like that is generally used to synchronize access to
			resources, like ensuring that a write to a buffer completes before reading from it, but it can also be used to transition image layouts and transfer queue family
			ownership when VK_SHARING_MODE_EXCLUSIVE (access of the object is limited to a single queue) is used. There is an equivalent buffer memory barrier to do this for buffers.
		*/
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		//The first two fields specify layout transitions. It is possible to use VK_IMAGE_LAYOUT_UNDEFINED as oldLayout if you don't care about the existing contents of the image.
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		/*
			If we are using the barrier to transfer queue family ownerships, then these two fields should be the indices of the queue families. They msut be set to VK_QUEUE_FAMILY_IGNORED
			if you don't want to do this, and not the default values.
		*/
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		//The image and subresourceRange specify the image that is affected and the specific part of the image. Our image is not an array and does not have any mipmapping levels, so only one level and layer are specified.
		barrier.image = m_Texture;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (HasStencilComponent(m_TextureFormat))
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = m_MipmapLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		/*
			Barriers are primarily used for synchronization purposes, so you must specify which types of operations that involve the resource must happen before the barrier, and
			which operations that involve the resource must wait on the barrier. We need to do that despite already using vkQueueWaitIdle to synchronize.

			There are 2 transitions that we need to handle in our case:
			- Undefined -> Transfer Destination: Transfer writes that don't need to wait on anything.
			- Transfer Destinations -> Shader Reading: Shader reads should wait on transfer writes, specifically the shader reads in the fragment shaders, because that's where we're going to use the texture.

			Transfer writes must occur in the pipeline transfer stage. Since the writers don't have to wait on anything, you may specify an empty access mask and the earlist possible pipeline
			stage VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT for the pre-barrier operations. It should be noted that the VK_PIPELINE_TRANSFER_BIT is not a real stage within the graphics and compute
			pipelines. It is more of a pseudo-stage where transfers happen.

			The image will be written in the same pipeline stage and be subsequently read in the fragment shader, which is why we specify shader reading access in the fragment shader
			pipeline stage. If we need to do more transitions in the future, then we will extend the function.

			One thing to note is that the command buffer submission results in implicit VK_ACCESS_HOST_WRITE_BIT synchronization in the beginning. Since the TransitionImageLayout
			function executes a command buffer with only a single command, you could use this implicit synchronization and set srcAccessMask to 0 if you ever need a VK_ACCESS_HOST_WRITE_BIT
			dependency in a layout transition. It is up to you if you want to be explicit about it or not, but personally lets not be fans of relying on these OpenGL like hidden functions.

			There is actually a special type of image layout that supports all operations, VK_IMAGE_LAYOUT_GENERAL. The problem with it is of course, it doesn't necessarily offer the best
			performance for any operation. It is required for some special cases, like using an image as both input and output, or for reading an image after it has left the preinitialized layout.

			All of the helper functions that submit commands so far has been set up to execute synchronously by waiting for the queue to become idle. For practical applications, it 
			is recommended to combine these operations into a single command buffer and execute them asynchronously for higher throughput, especially the transitions and copying in the CreateTextureImage function.
		*/
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		/*
			The depth buffer will be read from to perform depth tests to see if a fragment is visible, and will be written to when a new fragment is drawn. The reading happens
			in the VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT stage and the writing in the VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT. You should pick the earliest
			pipeline stage that matches the specified operations, so that it is ready for usage as depth attachment when it needs to be.
		*/
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			throw std::invalid_argument("Unsupported Layout Transition.");
		}

		/*
			All types of pipeline barriers are submitted using the same function. The first parameter after the command buffer specifies in which pipeline stage the operations occur 
			that should happen before the barrier.

			The second parameter specifies the pipeline stage in which operations will wait on the barrier. The pipeline stages that you are allowed to specify before and after the 
			barrier depends on how you use the resource before and after the barrier. For example, if you're going to read from a uniform after the barrier, you would
			specify a usage of VK_ACCESS_UNIFORM_READ_BIT and the earlist shader that will read from the uniform as pipeline stage, for example VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT.
			It would not make sense to specify a non-shader pipeline stage for this type of usage and the validation layers will warn you when you specify a pipeline stage that does not 
			match the type of usage.

			The third parameter is either 0 or VK_DEPENDENCY_BY_REGION_BIT. The latter turns the barrier into a per-region condition. This means that the implementation is allowed to 
			already begin reading from the parts of a resource that were written so far, for example.

			The last three pairs of parameters reference arrays of pipeline barriers of the three avaliable types: memory barriers, buffer memory barriers and image memory barriers like 
			the one we are using.
		*/
		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommands(commandBuffer, commandPool, *m_LogicalDevice, queue);
	}

	void VulkanTexture::GenerateMipmaps(VkImage texture, VkFormat imageFormat, int32_t textureWidth, int32_t textureHeight, uint32_t mipLevels)
	{
		/*
		    Mipmaps are precalculated, downscaled versions of an image. Each new image is half the width and height of the previous one. Mipmaps are used as a form of LOD (Level 
			of Detail). Objects that are far away from the camera will sample their textures from the smaller mip images. Using smaller images increases the rendering speed and avoids
			artifacts such as Moire patterns. In Vulkan, each of the mip images is stored in different mip levels of a VkImage. Mip level 0 is the original image, and the mip 
			levels after level 0 are commonly referred to as the mip chain.

			While it is very convenient to use a built-in function like vkCmdBlitImage to generate all the mip levels, it is unfortunately not guaranteed to be supported on 
			all platforms. It requires the texture image format we use to support linear filtering, which can be checked with the vkGetPhysicalDeviceFormatProperties function. 
		*/
		
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(*m_PhysicalDevice, imageFormat, &formatProperties);
		/*
			The VkFormatProperties struct has 3 fields named linearTilingFeatures, optimalTilingFeatures and bufferFeatures that each describe how the format can be used 
			depending on the way it is used. We created a texture image with the optimal tiling format, so we need to check optimalTilingFeatures. Support for the linear filtering 
			feature can be checked with VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT.

			There are two alternatives in this case. You could implement a function that searches common texture image formats for one that does support linear blitting, 
			or you could implement the mipmap generation in software with a library like stb_image_resize. Each mipmap level can then be loaded into the image in the same way that 
			you loaded the original image.

			It should be noted that it is uncommon in practice to generate the mipmap levels at runtime anyway. Usually, they are pregenerated and stored in the texture file alongside 
			the base level to improve loading speed.
		*/
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("Texture image format does not support linear filtering.\n");
		}

		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(*m_LogicalDevice, *m_CommandPool);

		VkImageMemoryBarrier barrier{};

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = texture;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		/*
			We're going to make several transitions, so we will reuse this VkImageMemoryBarrier. The fields set above will remain the same for all barriers. However, 
			subresourceRange.miplevel, oldLayout, newLayout, srcAccessMask and dstAccessMask will be changed for each transition.
		*/
		int32_t mipWidth = textureWidth;
		int32_t mipHeight = textureHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			//This loop will record each of the VkCmdBlitImage commands. Note that the loop variable starts at 1, not 0. 
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			//First, we transition level i - 1 to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL. This transition will wait for level i - 1 to be filled, either from the previous blit command or from vkCmdCopyBufferToImage.
			//The current blit command will wait on this transition.
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			/*
				Next, we specify the regions that will be used in the blit operation. The source mip level is i - 1 and the destination mip level is i. The two elements of 
				the srcOffsets array determine the 3D region that data will be blitted from. dstOffsets determine the region that data will be blitted to. The X and Y dimensions 
				of thje dstOffsets[1] are divided by 2 since each mip level is half the size of the previous level. The Z dimension of srcOffsets[1] and dstOffsets[1] must be 1, 
				since a 2D image has a depth of 1.
			*/

			VkImageBlit blitInfo{};
			blitInfo.srcOffsets[0] = { 0, 0, 0 };
			blitInfo.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitInfo.srcSubresource.mipLevel = i - 1;
			blitInfo.srcSubresource.baseArrayLayer = 0;
			blitInfo.srcSubresource.layerCount = 1;

			blitInfo.dstOffsets[0] = { 0, 0, 0 };
			blitInfo.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitInfo.dstSubresource.mipLevel = i;
			blitInfo.dstSubresource.baseArrayLayer = 0;
			blitInfo.dstSubresource.layerCount = 1;

			/*
				Now, we record the blit command. Note that texture is used for both the srcImage and dstImage parameter. This is because we're blitting between different levels of 
				the same image. The source mip level was just transitioned to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL and the destination level is still in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
				from texture creation.

				Beware if you are using a dedicated transfer queue. vkCmdBlitImage must be submitted to a queue with graphics capability in that case.

				The last parameter allows us to specify a VkFilter to use in the blit. We have the same filtering operations here that we had when making the sampler. Thus, 
				we use the VK_FILTER_LINEAR to enable interpolation.

			*/
			vkCmdBlitImage(commandBuffer, texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitInfo, VK_FILTER_LINEAR);

			/*
				This barrier transitions mip level i - 1 to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL. This transition waits on the current blit command to finish. All sampling 
				operations will wait on this transition to finish.
			*/
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			/*
				At the end of the loop, we divide the current mip dimensions by 2. We check each dimension before the division to ensure that the dimension never becomes 0. 
				This handles cases where the image is not square, since one of the mip dimensions would reach 1 before the other dimension. When this happens, that dimension should 
				remain 1 for all remaining levels.
			*/
			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		/*
			Before we end the command buffer, we insert one more pipeline barrier. This barrier transitions the last mip level from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL. This wasn't handled by the loop as the last mip level is never blitted from. 
		*/
		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommands(commandBuffer, *m_CommandPool, *m_LogicalDevice, *m_Queue);
	}
}	