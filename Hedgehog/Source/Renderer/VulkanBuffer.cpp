#include <Renderer/VulkanBuffer.h>

#include <Renderer/VulkanContext.h>
#include <Application/Application.h>


namespace Hedge
{

uint32_t FindMemoryType(VkPhysicalDevice device, uint32_t requiredType, VkMemoryPropertyFlags requiredProperties)
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

	for (uint32_t memoryType = 0; memoryType < memProperties.memoryTypeCount; memoryType++)
	{
		bool isRequiredType = (1 << memoryType) & requiredType;
		bool hasRequiredProperties = (memProperties.memoryTypes[memoryType].propertyFlags & requiredProperties) == requiredProperties;

		if (isRequiredType && hasRequiredProperties)
		{
			return memoryType;
		}
	}

	// We didn't find any suitable memory type, just end it
	assert(false);
	return 0;
}


VulkanVertexBuffer::VulkanVertexBuffer(const BufferLayout& layout,
									   const float* vertices,
									   unsigned int size)
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	this->layout = layout;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vulkanContext->device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
	{
		assert(false);
	}


	// Quary the memory requirements for the vertex buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vulkanContext->device, vertexBuffer, &memRequirements);

	// Allocate memory for the vertex buffer
	// TODO We're using CPU visible buffer for now, will be changed to use staging buffers
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(vulkanContext->chosenGPU,
											   memRequirements.memoryTypeBits,
											   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(vulkanContext->device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
	{
		assert(false);
	}

	vkBindBufferMemory(vulkanContext->device,
					   vertexBuffer,
					   vertexBufferMemory,
					   0); // offset into the block of memory, must be aligned according to the memRequirements.alignment


	// Copy data to the GPU
	void* data;
	vkMapMemory(vulkanContext->device, vertexBufferMemory, 0, size, 0, &data);
	memcpy(data, vertices, size);
	vkUnmapMemory(vulkanContext->device, vertexBufferMemory);
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	vkDestroyBuffer(vulkanContext->device, vertexBuffer, nullptr);
	vkFreeMemory(vulkanContext->device, vertexBufferMemory, nullptr);
}

void VulkanVertexBuffer::Bind(unsigned int slot) const
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	// TODO maybe this should be done in the vertex array, especiall if there are multiple vertex buffers
	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize vertexBufferOffsets[] = { 0 };
	vkCmdBindVertexBuffers(vulkanContext->commandBuffers[vulkanContext->frameInFlightIndex],
						   slot,
						   1, // bindingCount
						   vertexBuffers,
						   vertexBufferOffsets);
}

void VulkanVertexBuffer::SetData(const float* vertices, unsigned int size)
{
	// Not implemented
	assert(false);
}

} // namespace Hedge
