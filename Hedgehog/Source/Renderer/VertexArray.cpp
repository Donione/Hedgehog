#include <Renderer/VertexArray.h>

#include <Renderer/Renderer.h>
#include <Renderer/DirectX12VertexArray.h>
#include <Renderer/OpenGLVertexArray.h>
#include <Renderer/VulkanVertexArray.h>


namespace Hedge
{

void VertexArray::AddTexture(TextureType type, const std::shared_ptr<Texture>& texture)
{
	// TODO warn if adding a single texture when there are multiple textures of the same type described
	AddTexture(type, 0, texture);
}

void VertexArray::AddTexture(TextureType type,
							 const std::vector<std::shared_ptr<Texture>>& textures,
							 const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	auto indices = FindIndices(type, textureDescriptions);
	assert(indices.size() == textures.size());

	for (int i = 0; i < textures.size(); i++)
	{
		AddTexture(type, i, textures[i]);
	}
}

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

	case RendererAPI::API::Vulkan:
		return new VulkanVertexArray(inputShader, primitiveTopology, inputLayout, textureDescriptions);

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
