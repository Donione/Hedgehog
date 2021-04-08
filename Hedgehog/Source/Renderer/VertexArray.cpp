#include <Renderer/Renderer.h>
#include <Renderer/VertexArray.h>
#include <Renderer/DirectX12VertexArray.h>
#include <Renderer/OpenGLVertexArray.h>


namespace Hedge
{

VertexArray* VertexArray::Create(const std::shared_ptr<Shader>& inputShader,
								 PrimitiveTopology primitiveTopology, const BufferLayout& inputLayout,
								 const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLVertexArray(inputShader, primitiveTopology, textureDescriptions);

	case RendererAPI::API::DirectX12:
		return new DirectX12VertexArray(inputShader, primitiveTopology, inputLayout, textureDescriptions);

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}

int VertexArray::FindIndex(TextureType type, const std::vector<Hedge::TextureDescription>& textureDescriptions) const
{
	auto indices = FindIndices(type, textureDescriptions);
	assert(indices.size() == 1);
	return indices.front();
}

std::vector<int> VertexArray::FindIndices(TextureType type, const std::vector<Hedge::TextureDescription>& textureDescriptions) const
{
	std::vector<int> indices;
	int index = 0;

	for (auto it = textureDescriptions.begin(); it != textureDescriptions.end(); it++, index++)
	{
		if (it->type == type)
		{
			indices.push_back(index);
		}
	}

	return indices;
}

} // namespace Hedge
