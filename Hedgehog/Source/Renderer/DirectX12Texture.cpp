#include <Renderer/DirectX12Texture.h>

#include <Renderer/DirectX12Context.h>
#include <Application/Application.h>

#include <stb_image.h>


namespace Hedge
{

DirectX12Texture2D::DirectX12Texture2D(const std::string& filename)
	: filename(filename)
{
	int width;
	int height;
	int channels;

	stbi_set_flip_vertically_on_load(1);
	stbi_uc* data = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	this->width = width;
	this->height = height;

	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	// Describe and create a Texture2D.
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	dx12context->g_pd3dDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texture));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

	// Create the GPU upload buffer.
	heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	dx12context->g_pd3dDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap));

	// Copy data to the intermediate upload heap and then schedule a copy
	// from the upload heap to the Texture2D.
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = data;
	textureData.RowPitch = (long long)width * 4; // TexturePixelSize;
	textureData.SlicePitch = textureData.RowPitch * height;

	UpdateSubresources(dx12context->g_pd3dCommandList, texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
	auto resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	dx12context->g_pd3dCommandList->ResourceBarrier(1, &resBarrier);

	// I believe that the data are copied into the intermediate upload heap by this point and so it is safe to release here
	stbi_image_free(data);
}

} // namespace Hedge
