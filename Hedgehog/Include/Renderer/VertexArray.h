#pragma once

#include <memory>

#include <Renderer/Buffer.h>
#include <Renderer/Shader.h>
#include <Renderer/Texture.h>
#include <Model/Model.h>


namespace Hedge
{

// TODO
// multiple vertex/index buffers rendering
// maybe submit with name so it can be selected what will be rendered, defaulting to rendering everything in order
class VertexArray
{
public:
	virtual ~VertexArray() {}

	// TODO add a function that checks if the vertex array is ready to be drawn

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
	virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;
	virtual void AddTexture(TextureType type, const std::shared_ptr<Texture>& texture) = 0;
	virtual void AddTexture(TextureType type, int position, const std::shared_ptr<Texture>& texture) = 0;
	virtual void AddTexture(TextureType type, const std::vector<std::shared_ptr<Texture>>& textures) = 0;
	virtual void SetupGroups(const std::vector<VertexGroup>& groups) = 0;

	virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
	virtual const std::shared_ptr<IndexBuffer> GetIndexBuffer() const = 0;
	virtual const std::shared_ptr<Shader> GetShader() const = 0;
	virtual std::vector<std::pair<VertexGroup, float>>& GetGroups() = 0;

	static VertexArray* Create(const std::shared_ptr<Shader>& inputShader,
							   PrimitiveTopology primitiveTopology, const BufferLayout& inputLayout,
							   const std::vector<Hedge::TextureDescription>& textureDescriptions);

protected:
	int FindIndex(TextureType type, const std::vector<Hedge::TextureDescription>& textureDescriptions) const;
	std::vector<int> FindIndices(TextureType type, const std::vector<Hedge::TextureDescription>& textureDescriptions) const;
};

} // namespace Hedge