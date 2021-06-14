#include <Renderer/Renderer.h>

#include <Renderer/Buffer.h>
#include <Renderer/OpenGLBuffer.h>
#include <Renderer/DirectX12Buffer.h>
#include <Renderer/VulkanBuffer.h>


namespace Hedge
{

VertexBuffer* VertexBuffer::Create(const BufferLayout& layout,
								   const float* vertices,
								   unsigned int size)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLVertexBuffer(layout, vertices, size);

	case RendererAPI::API::DirectX12:
		return new DirectX12VertexBuffer(layout, vertices, size);
		
	case RendererAPI::API::Vulkan:
		return new VulkanVertexBuffer(layout, vertices, size);

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}

IndexBuffer* IndexBuffer::Create(const unsigned int* indices, unsigned int count)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLIndexBuffer(indices, count);

	case RendererAPI::API::DirectX12:
		return new DirectX12IndexBuffer(indices, count);

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}

BufferLayout BufferLayout::operator+(const BufferLayout& other) const
{
	BufferLayout result(*this);

	// The stride is unused in the complete buffer layout
	result.stride = 0;

	// The second buffer layout that is being added has a different input slot assigned
	// ASSUME creating the complete buffer layout by adding individual ones
	// is done in the same order as vertex buffers will be added to the vertex array
	unsigned int inputSlot = 0;
	if (!result.elements.empty())
	{
		inputSlot = result.elements.back().inputSlot + 1;
	}

	for (auto& element : other)
	{
		result.elements.push_back(element);
		result.elements.back().inputSlot = inputSlot;
	}

	return result;
}

BufferLayout& BufferLayout::operator+=(const BufferLayout& other)
{
	BufferLayout result(*this);

	result = result + other;
	*this = result;

	return *this;
}

void BufferLayout::CalculateOffsetsAndStride()
{
	unsigned int offset = 0;
	for (auto& element : elements)
	{
		// Element is offset by the sum of all previous elements' size
		element.offset = offset;

		offset += element.size;
	}

	// Stride is the sum of all elements' size
	stride = offset;
}

} // namespace Hedge
