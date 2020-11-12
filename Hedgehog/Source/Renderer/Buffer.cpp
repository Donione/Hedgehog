#include <Renderer/Renderer.h>
#include <Renderer/Buffer.h>


VertexBuffer* VertexBuffer::Create(const float* vertices, unsigned int size)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::OpenGL:
		return nullptr;

	case RendererAPI::None:
		return nullptr;

	default:
		return nullptr;
	}
}

IndexBuffer* IndexBuffer::Create(const unsigned int* indices, unsigned int count)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::OpenGL:
		return nullptr;

	case RendererAPI::None:
		return nullptr;

	default:
		return nullptr;
	}
}
