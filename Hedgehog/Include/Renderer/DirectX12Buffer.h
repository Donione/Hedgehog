#pragma once

#include <Renderer/Buffer.h>

#include <d3dx12.h>


namespace Hedge
{

class DirectX12VertexBuffer : public VertexBuffer
{


public:
	DirectX12VertexBuffer(PrimitiveTopology primitiveTopology,
						  const BufferLayout& layout,
						  const float* vertices,
						  unsigned int size);
	virtual ~DirectX12VertexBuffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override { /* do nothing */ }

	virtual const PrimitiveTopology GetPrimitiveType() const override { return primitiveTopology; }
	virtual const BufferLayout& GetLayout() const override { return layout; }

	const D3D12_VERTEX_BUFFER_VIEW* const GetView() const { return &vertexBufferView; }

private:
	D3D12_PRIMITIVE_TOPOLOGY GetDirectX12PrimitiveTopology(PrimitiveTopology type) const { return DirectX12PrimitiveTopologies[(int)type]; }


private:
	PrimitiveTopology primitiveTopology;
	BufferLayout layout;

	ID3D12Resource* vertexBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

	const D3D12_PRIMITIVE_TOPOLOGY DirectX12PrimitiveTopologies[4]
	{
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	};
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

} // namespace Hedge
