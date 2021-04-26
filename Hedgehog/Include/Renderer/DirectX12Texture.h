#pragma once

#include <Renderer/Texture.h>

#include <wrl.h>

#include <d3dx12.h>


namespace Hedge
{

class DirectX12Texture2D : public Texture2D
{
public:
	DirectX12Texture2D(const std::string& filename);
	virtual ~DirectX12Texture2D() { /* nothing to do */ }

	virtual void Bind(unsigned int slot = 0) const override { /* Do nothing */ };

	virtual unsigned int GetWidth() const override { return width; }
	virtual unsigned int GetHeight() const override { return height; }

	const D3D12_RESOURCE_DESC& GetDesc() const { return textureDesc; }
	ID3D12Resource* Get() const { return texture.Get(); }

private:
	std::string filename;

	unsigned int width;
	unsigned int height;

	D3D12_RESOURCE_DESC textureDesc{};
	Microsoft::WRL::ComPtr<ID3D12Resource> texture;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap;
};

} // namespace Hedge
