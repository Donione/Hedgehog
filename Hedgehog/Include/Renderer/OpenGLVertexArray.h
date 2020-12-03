#pragma once

#include <Renderer/VertexArray.h>

#include <glad/glad.h>


class OpenGLVertexArray : public VertexArray
{
public:
	OpenGLVertexArray();
	virtual ~OpenGLVertexArray();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
	virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

private:
	unsigned int GetOpenGLBaseType(ShaderDataType type) const { return OpenGLBaseTypes[(int)type]; }

private:
	unsigned int rendererID = 0;
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
