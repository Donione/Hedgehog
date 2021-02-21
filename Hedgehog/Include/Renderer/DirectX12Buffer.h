#pragma once

#include <Renderer/Buffer.h>

#include <d3dx12.h>


class DirectX12VertexBuffer : public VertexBuffer
{
public:
	DirectX12VertexBuffer(const BufferLayout& layout, const float* vertices, unsigned int size);
	virtual ~DirectX12VertexBuffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override { /* do nothing */ }

	virtual const BufferLayout& GetLayout() const override { return layout; }

	const D3D12_VERTEX_BUFFER_VIEW* const GetView() const { return &vertexBufferView; }

private:
	BufferLayout layout;

	ID3D12Resource* vertexBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
};


class DirectX12IndexBuffer : public IndexBuffer
{
public:
	DirectX12IndexBuffer(const unsigned int* indices, unsigned int count);
	virtual ~DirectX12IndexBuffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override { /* do nothing */ }

	virtual unsigned int GetCount() const override { return count; };

	const D3D12_INDEX_BUFFER_VIEW* GetView() const { return &indexBufferView; }

private:
	unsigned int count = 0;

	ID3D12Resource* indexBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
};
