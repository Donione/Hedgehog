#pragma once

class VertexBuffer
{
public:
	virtual ~VertexBuffer() {}

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	static VertexBuffer* Create(const float* vertices, unsigned int size);
};

class IndexBuffer
{
public:
	virtual ~IndexBuffer() {}

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual unsigned int GetCount() const = 0;

	static IndexBuffer* Create(const unsigned int* indices, unsigned int count);
};
