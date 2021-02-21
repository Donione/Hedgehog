#include <Renderer/Renderer.h>
#include <Renderer/VertexArray.h>
#include <Renderer/DirectX12VertexArray.h>
#include <Renderer/OpenGLVertexArray.h>


VertexArray* VertexArray::Create(const std::shared_ptr<Shader>& inputShader,
								 const BufferLayout& inputLayout,
								 const std::shared_ptr<Texture>& inputTexture)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLVertexArray(inputShader, inputTexture);

	case RendererAPI::API::DirectX12:
		return new DirectX12VertexArray(inputShader, inputLayout, inputTexture);

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}
