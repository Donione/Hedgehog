#include <Renderer/DirectX12VertexArray.h>

#include <Application/Application.h>
#include <Renderer/DirectX12Context.h>
#include <Renderer/DirectX12Texture.h>

#include <vector>


namespace Hedge
{

DirectX12VertexArray::DirectX12VertexArray(const std::shared_ptr<Shader>& inputShader,
										   PrimitiveTopology primitiveTopology, const BufferLayout& inputLayout,
										   const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	// TODO for fun, see how the ref count changes
	this->shader = std::dynamic_pointer_cast<DirectX12Shader>(inputShader);
	this->primitiveTopology = primitiveTopology;
	this->bufferLayout = inputLayout;
	this->textureDescriptions = textureDescriptions;
	textures.resize(textureDescriptions.size());

	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	std::vector<CD3DX12_ROOT_PARAMETER> rootParameters;
	rootParameters.resize(shader->GetConstBufferCount());

	for (int i = 0; i < shader->GetConstBufferCount(); i++)
	{
		// TODO add shader visibility to const buffer description
		rootParameters[i].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_ALL);
	}

	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
	if (!textureDescriptions.empty())
	{
		// The textures' description table goes just after the CBVs
		texturesRootParamIndex = (unsigned int)rootParameters.size();

		CD3DX12_DESCRIPTOR_RANGE ranges[1] = {};
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, (UINT)textureDescriptions.size(), 0, 0);
		CD3DX12_ROOT_PARAMETER param;
		param.InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters.push_back(param);

		staticSamplers.resize(1);
		staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].MipLODBias = 0;
		staticSamplers[0].MaxAnisotropy = 0;
		staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		staticSamplers[0].MinLOD = 0.0f;
		staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
		staticSamplers[0].ShaderRegister = 0;
		staticSamplers[0].RegisterSpace = 0;
		staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CreateSRVHeap();
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init((UINT)rootParameters.size(), rootParameters.data(),
						   (UINT)staticSamplers.size(), staticSamplers.empty() ? nullptr : staticSamplers.data(),
						   rootSignatureFlags);

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	dx12context->g_pd3dDevice->CreateRootSignature(0,
												   signature->GetBufferPointer(),
												   signature->GetBufferSize(),
												   IID_PPV_ARGS(&m_rootSignature));

	CreatePSO();
}

DirectX12VertexArray::~DirectX12VertexArray()
{
}

void DirectX12VertexArray::Bind() const
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	dx12context->g_pd3dCommandList->SetPipelineState(m_pipelineState[currentPSO].Get());
	dx12context->g_pd3dCommandList->SetGraphicsRootSignature(m_rootSignature.Get());

	if (!textures.empty())
	{
		ID3D12DescriptorHeap* ppHeaps[] = { srvHeap.Get() };
		dx12context->g_pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		dx12context->g_pd3dCommandList->SetGraphicsRootDescriptorTable(texturesRootParamIndex, srvHeap->GetGPUDescriptorHandleForHeapStart());
	}

	shader->Bind();

	dx12context->g_pd3dCommandList->IASetPrimitiveTopology(GetDirectX12PrimitiveTopology(primitiveTopology));

	unsigned int slot = 0;
	for (auto& vertexBuffer : vertexBuffers)
	{
		vertexBuffer->Bind(slot++);
	}

	indexBuffer->Bind();
}

void DirectX12VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	vertexBuffers.push_back(vertexBuffer);
}

void DirectX12VertexArray::AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
	this->indexBuffer = indexBuffer;
}

void DirectX12VertexArray::AddTexture(TextureType type, const std::shared_ptr<Texture>& texture)
{
	// TODO warn if adding a single texture when there are multiple textures of the same type described
	AddTexture(type, 0, texture);
}

void DirectX12VertexArray::AddTexture(TextureType type, int position, const std::shared_ptr<Texture>& texture)
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	auto indices = FindIndices(type, textureDescriptions);
	assert(indices.size() >= (position + 1));
	int index = indices[position];
	textures[index] = texture;

	// Describe and create SRV for the texture and put it on the SRV heap.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = std::dynamic_pointer_cast<DirectX12Texture2D>(texture)->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(),
											index,
											dx12context->g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	dx12context->g_pd3dDevice->CreateShaderResourceView(std::dynamic_pointer_cast<DirectX12Texture2D>(texture)->Get(), &srvDesc, cpuHandle);
}

void DirectX12VertexArray::AddTexture(TextureType type, const std::vector<std::shared_ptr<Texture>>& textures)
{
	auto indices = FindIndices(type, textureDescriptions);
	assert(indices.size() == textures.size());

	for (int i = 0; i < textures.size(); i++)
	{
		AddTexture(type, i, textures[i]);
	}
}

void DirectX12VertexArray::SetupGroups(const std::vector<VertexGroup>& groups)
{
	for (auto& group : groups)
	{
		this->groups.emplace_back(group, 0.0f);
	}
}

void DirectX12VertexArray::UpdateRenderSettings()
{
	CreatePSO();
}

void DirectX12VertexArray::CreatePSO()
{
	//D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputLayout.Size()];
	// All the cool kinds use smart pointers these days insted of new/delete
	//auto inputElementDescs = std::make_unique<D3D12_INPUT_ELEMENT_DESC[]>(inputLayout.Size());
	// or just use a vector
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;

	//unsigned int inputIndex = 0;
	unsigned int offset = 0;
	for (auto& input : bufferLayout)
	{
		// TODO learn what all of these do
		//SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, InstanceDataStepRate
		inputElementDescs.push_back(
			{
				input.name.c_str(),
				0,
				GetDirectXFormat(input.type),
				input.inputSlot,
				(UINT)input.offset,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				0
			});
	}

	auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.DepthEnable = RenderCommand::GetDepthTest(); // depthStencilEnabled;
	// TODO what IS a stencil ?

	auto blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blendDesc.RenderTarget[0].BlendEnable = RenderCommand::GetBlending();
	blendDesc.RenderTarget[0].LogicOpEnable = false;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs.data(), (UINT)inputElementDescs.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = shader->GetVSBytecode();
	psoDesc.PS = shader->GetPSBytecode();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = RenderCommand::GetFaceCulling() ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FillMode = RenderCommand::GetWireframeMode() ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	psoDesc.BlendState = blendDesc; // CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleMask = UINT_MAX; // sample mask has to do with multi-sampling. 0xFFFFFFFF (UINT_MAX) means point sampling is done
	psoDesc.PrimitiveTopologyType = GetPipelinePrimitiveTopology(primitiveTopology);
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1; // This must be the same as the SampleDesc for swap chain
	psoDesc.SampleDesc.Quality = 0;

	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
	currentPSO = (currentPSO + 1) % MAX_PSOS;
	dx12context->g_pd3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState[currentPSO])); // TODO handle fail

	// smart pointers or vectors clean themselves up
	//delete[] inputElementDescs;
}

void DirectX12VertexArray::CreateSRVHeap()
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	// Describe and create a shader resource view (SRV) heap for the textures.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = (UINT)textureDescriptions.size();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	dx12context->g_pd3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
}

} // namespace Hedge
