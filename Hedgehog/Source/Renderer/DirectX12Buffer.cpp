#include <Renderer/DirectX12Buffer.h>

#include <Renderer/DirectX12Context.h>
#include <Application/Application.h>


namespace Hedge
{

DirectX12VertexBuffer::DirectX12VertexBuffer(PrimitiveTopology primitiveTopology,
											 const BufferLayout& layout,
											 const float* vertices,
											 unsigned int size)
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
	assert(dx12context);

	this->primitiveTopology = primitiveTopology;
	this->layout = layout;

	// TODO use default upload heap and copy data into it using upload heap
	// this needs a barrier and command list execution
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	dx12context->g_pd3dDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer));

	UINT8* pVertexDataBegin = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	memcpy(pVertexDataBegin, vertices, size);
	vertexBuffer->Unmap(0, nullptr);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = layout.GetStride();
	vertexBufferView.SizeInBytes = size;
}

DirectX12VertexBuffer::~DirectX12VertexBuffer()
{
	if (vertexBuffer) vertexBuffer->Release();
	vertexBuffer = nullptr;
	vertexBufferView = {};
}

void DirectX12VertexBuffer::Bind() const
{
	assert(vertexBuffer);

	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	dx12context->g_pd3dCommandList->IASetPrimitiveTopology(GetDirectX12PrimitiveTopology(primitiveTopology));
	dx12context->g_pd3dCommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void DirectX12VertexBuffer::SetData(const float* vertices, unsigned int size)
{
	assert(size == vertexBufferView.SizeInBytes);

	UINT8* pVertexDataBegin = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	memcpy(pVertexDataBegin, vertices, size);
	vertexBuffer->Unmap(0, nullptr);
}


DirectX12IndexBuffer::DirectX12IndexBuffer(const unsigned int* indices, unsigned int count)
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
	assert(dx12context);

	this->count = count;
	const unsigned int size = count * sizeof(unsigned int);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	dx12context->g_pd3dDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)); // TODO handle fail

	UINT8* pIndexDataBegin = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)); // TODO handle fail
	memcpy(pIndexDataBegin, indices, size);
	indexBuffer->Unmap(0, nullptr);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = size;
}

DirectX12IndexBuffer::~DirectX12IndexBuffer()
{
	if (indexBuffer) indexBuffer->Release();
	indexBuffer = nullptr;
	indexBufferView = {};
}

void DirectX12IndexBuffer::Bind() const
{
	assert(indexBuffer);

	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	dx12context->g_pd3dCommandList->IASetIndexBuffer(&indexBufferView);
}

} // namespace Hedge
