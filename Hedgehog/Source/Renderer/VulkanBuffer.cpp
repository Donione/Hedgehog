#include <Renderer/VulkanBuffer.h>

#include <Renderer/VulkanContext.h>
#include <Application/Application.h>


namespace Hedge
{

VulkanVertexBuffer::VulkanVertexBuffer(const BufferLayout& layout,
									   const float* vertices,
									   unsigned int size)
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	this->layout = layout;

	vulkanContext->CreateStagedBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
									  (void*)vertices,
									  size,
									  &vertexBuffer,
									  &vertexBufferMemory);
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	vulkanContext->DestoyVulkanBuffer(vertexBuffer, vertexBufferMemory);
}

void VulkanVertexBuffer::Bind(unsigned int slot) const
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	// TODO maybe this should be done in the vertex array, especiall if there are multiple vertex buffers
	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize vertexBufferOffsets[] = { 0 };
	vkCmdBindVertexBuffers(vulkanContext->commandBuffers[vulkanContext->swapChainImageIndex],
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

	vulkanContext->CreateStagedBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
									  (void*)indices,
									  size,
									  &indexBuffer,
									  &indexBufferMemory);
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	vulkanContext->DestoyVulkanBuffer(indexBuffer, indexBufferMemory);
}

void VulkanIndexBuffer::Bind() const
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	vkCmdBindIndexBuffer(vulkanContext->commandBuffers[vulkanContext->swapChainImageIndex],
						 indexBuffer,
						 0, // offset
						 VK_INDEX_TYPE_UINT32);
}

} // namespace Hedge
