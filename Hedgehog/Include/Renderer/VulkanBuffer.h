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

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
};

} // namespace Hedge
