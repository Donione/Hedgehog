#pragma once

#include <Renderer/Buffer.h>

#include <vulkan/vulkan.h>


namespace Hedge
{

class VulkanVertexBuffer : public VertexBuffer
{
public:
	VulkanVertexBuffer(const BufferLayout& layout,
					   const float* vertices,
					   unsigned int size);
	virtual ~VulkanVertexBuffer() override;

	virtual void Bind(unsigned int slot = 0) const override;
	virtual void Unbind() const override { /* Do Nothing */ }

	virtual const BufferLayout& GetLayout() const override { return layout; }

	virtual void SetData(const float* vertices, unsigned int size) override;

private:
	BufferLayout layout;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
};


class VulkanIndexBuffer : public IndexBuffer
{
public:
	VulkanIndexBuffer(const unsigned int* indices, unsigned int count);
	virtual ~VulkanIndexBuffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override {}

	virtual unsigned int GetCount() const override { return count; }

private:
	unsigned int count = 0;

	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
};

} // namespace Hedge
