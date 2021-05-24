#pragma once

#include <Renderer/Buffer.h>


namespace Hedge
{

class OpenGLVertexBuffer : public VertexBuffer
{
public:
	OpenGLVertexBuffer(const BufferLayout& layout,
					   const float* vertices,
					   unsigned int size);
	virtual ~OpenGLVertexBuffer() override;

	virtual void Bind(unsigned int slot = 0) const override;
	virtual void Unbind() const override;


	virtual const BufferLayout& GetLayout() const override { return layout; }

	virtual void SetData(const float* vertices, unsigned int size) override;

private:
	unsigned int rendererID = 0;
	BufferLayout layout;
};


class OpenGLIndexBuffer : public IndexBuffer
{
public:
	OpenGLIndexBuffer(const unsigned int* indices, unsigned int count);
	virtual ~OpenGLIndexBuffer();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual unsigned int GetCount() const override { return count; };

private:
	unsigned int rendererID;
	unsigned int count = 0;
};

} // namespace Hedge
