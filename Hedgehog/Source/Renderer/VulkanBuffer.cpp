#include <Renderer/VulkanBuffer.h>

#include <Renderer/VulkanContext.h>
#include <Application/Application.h>


namespace Hedge
{

uint32_t FindMemoryType(VkPhysicalDevice device, uint32_t requiredType, VkMemoryPropertyFlags requiredProperties)
{
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

void CreateVulkanBuffer(VkDevice device,
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
					VkMemoryPropertyFlags requiredProperties,
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
											   requiredProperties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory) != VK_SUCCESS)
	{
		assert(false);
	}
}

void CopyBuffer(VkDevice device,
				VkCommandPool commandPool,
				VkQueue queue,
				VkBuffer srcBuffer,
				VkBuffer dstBuffer,
				VkDeviceSize size)
{
	// TODO temporary create a new commandBuffer and submit it here
	// We want to move it to RendererAPI::Begin() so we can submit multiple transfers and all that good stuff
	// Possibly creating a separate command pool amd queue dedicate for transfers

	// Create a one-shot command buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// Record a transfer command
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	// Submit the command buffer and wait for completion
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	// Clean up the command buffer
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void CreateStagingBuffer(VkDevice device,
						 VkPhysicalDevice physicalDevice,
						 VkDeviceSize size,
						 VkBuffer* buffer,
						 VkDeviceMemory* bufferMemory)
{
	CreateVulkanBuffer(device,
					   size,
					   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					   buffer);

	AllocateMemory(device,
				   physicalDevice,
				   *buffer,
				   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				   bufferMemory);

	vkBindBufferMemory(device,
					   *buffer,
					   *bufferMemory,
					   0); // offset into the block of memory, must be aligned according to the memRequirements.alignment
}

void CreateBuffer(VkDevice device,
				  VkPhysicalDevice physicalDevice,
				  VkCommandPool commandPool,
				  VkQueue queue,
				  VkBufferUsageFlags usage,
				  void* data,
				  VkDeviceSize size,
				  VkBuffer* buffer,
				  VkDeviceMemory* bufferMemory)
{
	// Create a staging buffer
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	CreateStagingBuffer(device,
						physicalDevice,
						size,
						&stagingBuffer,
						&stagingBufferMemory);

	// Copy data to the staging buffer
	void* mappedData;
	vkMapMemory(device, stagingBufferMemory, 0, size, 0, &mappedData);
	memcpy(mappedData, data, size);
	vkUnmapMemory(device, stagingBufferMemory);

	// Create buffer on GPU
	CreateVulkanBuffer(device,
					   size,
					   VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
					   buffer);

	AllocateMemory(device,
				   physicalDevice,
				   *buffer,
				   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				   bufferMemory);

	vkBindBufferMemory(device,
					   *buffer,
					   *bufferMemory,
					   0); // offset into the block of memory, must be aligned according to the memRequirements.alignment

	CopyBuffer(device,
			   commandPool,
			   queue,
			   stagingBuffer,
			   *buffer,
			   size);

	// Clean up the staging buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}


VulkanVertexBuffer::VulkanVertexBuffer(const BufferLayout& layout,
									   const float* vertices,
									   unsigned int size)
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	this->layout = layout;

	CreateBuffer(vulkanContext->device,
				 vulkanContext->chosenGPU,
				 vulkanContext->commandPool,
				 vulkanContext->graphicsQueue,
				 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 (void*)vertices,
				 size,
				 &vertexBuffer,
				 &vertexBufferMemory);
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
				 vulkanContext->chosenGPU,
				 vulkanContext->commandPool,
				 vulkanContext->graphicsQueue,
				 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				 (void*)indices,
				 size,
				 &indexBuffer,
				 &indexBufferMemory);
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
