#pragma once

#include <Renderer/VertexArray.h>

#include <Renderer/OpenGLShader.h>

#include <glad/glad.h>


namespace Hedge
{

class OpenGLVertexArray : public VertexArray
{
public:
	OpenGLVertexArray(const std::shared_ptr<Shader>& inputShader,
					  const std::shared_ptr<Texture>& inputTexture);
	virtual ~OpenGLVertexArray() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
	virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

	virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return vertexBuffers; }
	virtual const std::vector<std::shared_ptr<IndexBuffer>>& GetIndexBuffer() const override { return indexBuffers; }
	virtual const std::shared_ptr<Shader> GetShader() const override { return shader; }
	virtual const std::shared_ptr<Texture>& GetTexture() const override { return texture; }

private:
	unsigned int GetOpenGLBaseType(ShaderDataType type) const { return OpenGLBaseTypes[(int)type]; }

private:
	unsigned int rendererID = 0;
	std::shared_ptr<OpenGLShader> shader;
	std::shared_ptr<Texture> texture;
	std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
	std::vector<std::shared_ptr<IndexBuffer>> indexBuffers;

	// Array of OpenGL base types corresponding to ShaderDataType
	// TODECIDE map, switch better?
	const unsigned int OpenGLBaseTypes[10] =
	{
		GL_NONE,
		GL_FLOAT,
		GL_FLOAT,
		GL_FLOAT,
		GL_FLOAT,
		GL_INT,
		GL_INT,
		GL_INT,
		GL_INT,
		GL_BOOL
	};
};

} // namespace Hedge
