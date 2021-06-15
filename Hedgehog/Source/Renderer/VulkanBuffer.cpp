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

void CreateBuffer(VkDevice device,
				  VkDeviceSize size,
				  VkBufferUsageFlags usage,
				  VkBuffer* buffer)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, buffer) != VK_SUCCESS)
	{
		assert(false);
	}
}

void AllocateMemory(VkDevice device,
					VkPhysicalDevice physicalDevice,
					VkBuffer buffer,
					VkDeviceMemory* bufferMemory)
{
	// Query the memory requirements for the vertex buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	// Allocate memory for the vertex buffer
	// TODO We're using CPU visible buffer for now, will be changed to use staging buffers
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice,
											   memRequirements.memoryTypeBits,
											   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory) != VK_SUCCESS)
	{
		assert(false);
	}
}


VulkanVertexBuffer::VulkanVertexBuffer(const BufferLayout& layout,
									   const float* vertices,
									   unsigned int size)
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	this->layout = layout;

	CreateBuffer(vulkanContext->device,
				 size,
				 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 &vertexBuffer);

	AllocateMemory(vulkanContext->device,
				   vulkanContext->chosenGPU,
				   vertexBuffer,
				   &vertexBufferMemory);

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

	if (vertexBuffer != VK_NULL_HANDLE) vkDestroyBuffer(vulkanContext->device, vertexBuffer, nullptr);
	if (vertexBufferMemory != VK_NULL_HANDLE) vkFreeMemory(vulkanContext->device, vertexBufferMemory, nullptr);
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


VulkanIndexBuffer::VulkanIndexBuffer(const unsigned int* indices, unsigned int count)
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	this->count = count;
	VkDeviceSize size = count * sizeof(unsigned int);

	CreateBuffer(vulkanContext->device,
				 size,
				 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				 &indexBuffer);

	AllocateMemory(vulkanContext->device,
				   vulkanContext->chosenGPU,
				   indexBuffer,
				   &indexBufferMemory);

	vkBindBufferMemory(vulkanContext->device,
					   indexBuffer,
					   indexBufferMemory,
					   0); // offset into the block of memory, must be aligned according to the memRequirements.alignment

	// Copy data to the GPU
	void* data;
	vkMapMemory(vulkanContext->device, indexBufferMemory, 0, size, 0, &data);
	memcpy(data, indices, size);
	vkUnmapMemory(vulkanContext->device, indexBufferMemory);
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	if (indexBuffer != VK_NULL_HANDLE) vkDestroyBuffer(vulkanContext->device, indexBuffer, nullptr);
	if (indexBufferMemory != VK_NULL_HANDLE) vkFreeMemory(vulkanContext->device, indexBufferMemory, nullptr);
}

void VulkanIndexBuffer::Bind() const
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	vkCmdBindIndexBuffer(vulkanContext->commandBuffers[vulkanContext->frameInFlightIndex],
						 indexBuffer,
						 0, // offset
						 VK_INDEX_TYPE_UINT32);
}

} // namespace Hedge
