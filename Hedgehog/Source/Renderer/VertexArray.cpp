#include <Renderer/Renderer.h>
#include <Renderer/VertexArray.h>


VertexArray* VertexArray::Create()
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
