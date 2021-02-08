#include <Renderer/DirectX12Buffer.h>

#include <Renderer/DirectX12Context.h>
#include <Application/Application.h>


DirectX12VertexBuffer::DirectX12VertexBuffer(const BufferLayout& layout, const float* vertices, unsigned int size)
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
	assert(dx12context);

	this->layout = layout;

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


DirectX12IndexBuffer::DirectX12IndexBuffer(const unsigned int* indices, unsigned int count)
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
	assert(dx12context);

	this->count = count;
	const unsigned int size = count * sizeof(unsigned int);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	ThrowIfFailed(dx12context->g_pd3dDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)));

	UINT8* pIndexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
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
