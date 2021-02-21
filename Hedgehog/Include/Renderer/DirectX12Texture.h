#pragma once

#include <Renderer/Texture.h>

#include <wrl.h>

#include <d3d12.h>


class DirectX12Texture2D : public Texture2D
{
public:
	DirectX12Texture2D(const std::string& filename);
	virtual ~DirectX12Texture2D() { /* nothing to do */ }

	virtual void Bind(unsigned int slot = 0) const override;

	virtual unsigned int GetWidth() const override { return width; }
	virtual unsigned int GetHeight() const override { return height; }

	void SetRootParamIndex(unsigned int index) { rootParamIndex = index; }

private:
	std::string filename;

	unsigned int width;
	unsigned int height;

	unsigned int rootParamIndex = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> texture;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
};
