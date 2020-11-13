#pragma once

#include <Renderer/Buffer.h>


class OpenGLVertexBuffer : public VertexBuffer
{
public:
	OpenGLVertexBuffer(const float* vertices, unsigned int size);
	virtual ~OpenGLVertexBuffer();

	virtual void Bind() const override;
	virtual void Unbind() const override;

private:
	unsigned int rendererID = 0;
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
