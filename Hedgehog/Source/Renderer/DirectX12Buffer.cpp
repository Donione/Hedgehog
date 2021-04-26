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

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	dx12context->g_pd3dDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	dx12context->g_pd3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferUploadHeap));

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<const void*>(vertices);
	vertexData.RowPitch = size;
	vertexData.SlicePitch = vertexData.RowPitch;

	UpdateSubresources(dx12context->g_pd3dCommandList, vertexBuffer.Get(), vertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);

	auto resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	dx12context->g_pd3dCommandList->ResourceBarrier(1, &resBarrier);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = layout.GetStride();
	vertexBufferView.SizeInBytes = size;
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

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	dx12context->g_pd3dDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)); // TODO handle fail

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	dx12context->g_pd3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUploadHeap));

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<const void*>(indices);
	indexData.RowPitch = size;
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(dx12context->g_pd3dCommandList, indexBuffer.Get(), indexBufferUploadHeap.Get(), 0, 0, 1, &indexData);

	auto resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	dx12context->g_pd3dCommandList->ResourceBarrier(1, &resBarrier);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = size;
}

void DirectX12IndexBuffer::Bind() const
{
	assert(indexBuffer);

	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	dx12context->g_pd3dCommandList->IASetIndexBuffer(&indexBufferView);
}

} // namespace Hedge
