#include <Renderer/Renderer.h>

#include <Renderer/Buffer.h>
#include <Renderer/OpenGLBuffer.h>


VertexBuffer* VertexBuffer::Create(const float* vertices, unsigned int size)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLVertexBuffer(vertices, size);

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
