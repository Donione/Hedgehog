#pragma once

#include <Renderer/VertexArray.h>

#include <Renderer/DirectX12Shader.h>

#include <wrl.h>
#include <d3dx12.h>


namespace Hedge
{

// TODO rename to something else, maybe something pipeline-y
class DirectX12VertexArray : public VertexArray
{
public:
	DirectX12VertexArray(const std::shared_ptr<Shader>& inputShader,
						 PrimitiveTopology primitiveTopology, const BufferLayout& inputLayout,
						 const std::vector<Hedge::TextureDescription>& textureDescriptions);
	virtual ~DirectX12VertexArray() override;

	virtual void Bind() const override;
	virtual void Unbind() const override { /* do nothing */ }

	virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
	virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
	virtual void AddTexture(TextureType type, const std::shared_ptr<Texture>& texture) override {}
	virtual void AddTexture(TextureType type, int position, const std::shared_ptr<Texture>& texture) override {}
	virtual void AddTexture(TextureType type, const std::vector<std::shared_ptr<Texture>>& textures) override {}

	virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return vertexBuffers; }
	virtual const std::vector<std::shared_ptr<IndexBuffer>>& GetIndexBuffer() const override { return indexBuffers; }
	// TODO why doesn't returning a shared_ptr reference work when upcasting?
	virtual const std::shared_ptr<Shader> GetShader() const override { return shader; }

	void UpdateRenderSettings();

private:
	DXGI_FORMAT GetDirectXFormat(ShaderDataType type) const { return DirectXFormats[(int)type]; }
	D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPipelinePrimitiveTopology(PrimitiveTopology topology) const { return pipelinePrimitiveTopologies[(int)topology]; }

	void CreatePSO();
	void CreateSRVHeap();

private:
	// TODO this should be just the number of frames in flight
	// increasing this as a workaround for multiple PSO updates per frame
	static const int MAX_PSOS = 32;
	int currentPSO = -1;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	// We're keeping a circular buffer of PSOs because when a render setting changes
	// we have some frames in-flight with the old PSO bound
	// TODO For some reason waiting for the last frame and then releasing the current PSO didn't work
	// I was still getting a GPU error, deletion of a live object (the PSO)
	// So keeping a buffer the size of the number of frames in flight should be enough for the old PSOs
	// to get out of the GPU command queue
	// TODO Investigate how to properly wait for the PSO to be unbound
	//      Is it because of the command allocator/queue or because the command list is recording?
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState[MAX_PSOS];

	std::shared_ptr<DirectX12Shader> shader;
	PrimitiveTopology primitiveTopology;
	BufferLayout bufferLayout;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
	unsigned int texturesRootParamIndex;
	std::shared_ptr<Texture> texture;
	std::shared_ptr<Texture> normalMap;
	std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
	std::vector<std::shared_ptr<IndexBuffer>> indexBuffers;

	// TODO check all of these
	const DXGI_FORMAT DirectXFormats[10] =
	{
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32G32_SINT,
		DXGI_FORMAT_R32G32B32_SINT,
		DXGI_FORMAT_R32G32B32A32_SINT,
		DXGI_FORMAT_R8_UINT
	};

	const D3D12_PRIMITIVE_TOPOLOGY_TYPE pipelinePrimitiveTopologies[4]
	{
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	};
};

} // namespace Hedge
