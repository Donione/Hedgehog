#include <Renderer/Renderer.h>

#include <Renderer/Buffer.h>
#include <Renderer/OpenGLBuffer.h>
#include <Renderer/DirectX12Buffer.h>


namespace Hedge
{

VertexBuffer* VertexBuffer::Create(const BufferLayout& layout, const float* vertices, unsigned int size)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLVertexBuffer(layout, vertices, size);

	case RendererAPI::API::DirectX12:
		return new DirectX12VertexBuffer(layout, vertices, size);

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
