#include <Renderer/Renderer.h>
#include <Renderer/VertexArray.h>
#include <Renderer/OpenGLVertexArray.h>


VertexArray* VertexArray::Create(const std::shared_ptr<Shader>& inputShader,
								 const BufferLayout& inputLayout)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLVertexArray(inputShader);

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}
