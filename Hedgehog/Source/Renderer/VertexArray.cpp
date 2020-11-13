#include <Renderer/Renderer.h>
#include <Renderer/VertexArray.h>
#include <Renderer/OpenGLVertexArray.h>


VertexArray* VertexArray::Create()
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::OpenGL:
		return new OpenGLVertexArray();

	case RendererAPI::None:
		return nullptr;

	default:
		return nullptr;
	}
}
