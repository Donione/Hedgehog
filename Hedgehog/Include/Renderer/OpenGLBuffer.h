#pragma once

#include <Renderer/Buffer.h>


namespace Hedge
{

class OpenGLVertexBuffer : public VertexBuffer
{
public:
	OpenGLVertexBuffer(PrimitiveTopology primitiveTopology,
					   const BufferLayout& layout,
					   const float* vertices,
					   unsigned int size);
	virtual ~OpenGLVertexBuffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;


	virtual const PrimitiveTopology GetPrimitiveType() const override { return primitiveTopology; }
	virtual const BufferLayout& GetLayout() const override { return layout; }

private:
	unsigned int rendererID = 0;
	PrimitiveTopology primitiveTopology;
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
