#pragma once

#include <memory>

#include <Renderer/Buffer.h>
#include <Renderer/Shader.h>
#include <Renderer/Texture.h>


// TODO
// multiple vertex/index buffers rendering
// maybe submit with name so it can be selected what will be rendered, defaulting to rendering everything in order
class VertexArray
{
public:
	virtual ~VertexArray() {}

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
	virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

	virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
	virtual const std::vector<std::shared_ptr<IndexBuffer>>& GetIndexBuffer() const = 0;
	virtual const std::shared_ptr<Shader> GetShader() const = 0;
	virtual const std::shared_ptr<Texture>& GetTexture() const = 0;

	static VertexArray* Create(const std::shared_ptr<Shader>& inputShader,
							   const BufferLayout& inputLayout,
							   const std::shared_ptr<Texture>& inputTexture = nullptr);
};