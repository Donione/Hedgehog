#include <Renderer/Renderer.h>
#include <Renderer/VertexArray.h>
#include <Renderer/OpenGLVertexArray.h>


VertexArray* VertexArray::Create()
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLVertexArray();

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}
