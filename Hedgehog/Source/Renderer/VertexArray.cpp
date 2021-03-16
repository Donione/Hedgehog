#include <Renderer/Renderer.h>
#include <Renderer/VertexArray.h>
#include <Renderer/DirectX12VertexArray.h>
#include <Renderer/OpenGLVertexArray.h>


namespace Hedge
{

VertexArray* VertexArray::Create(const std::shared_ptr<Shader>& inputShader,
								 PrimitiveTopology primitiveTopology, const BufferLayout& inputLayout,
								 const std::shared_ptr<Texture>& inputTexture)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLVertexArray(inputShader, primitiveTopology, inputTexture);

	case RendererAPI::API::DirectX12:
		return new DirectX12VertexArray(inputShader, primitiveTopology, inputLayout, inputTexture);

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}

} // namespace Hedge
