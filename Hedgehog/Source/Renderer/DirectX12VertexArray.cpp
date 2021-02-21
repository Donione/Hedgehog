#include <Renderer/DirectX12VertexArray.h>

#include <Application/Application.h>
#include <Renderer/DirectX12Context.h>
#include <Renderer/DirectX12Texture.h>

#include <vector>


DirectX12VertexArray::DirectX12VertexArray(const std::shared_ptr<Shader>& inputShader,
										   const BufferLayout& inputLayout,
										   const std::shared_ptr<Texture>& inputTexture)
{
	// TODO for fun, see how the ref count changes
	shader = std::dynamic_pointer_cast<DirectX12Shader>(inputShader);
	texture = inputTexture;

	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	//CD3DX12_DESCRIPTOR_RANGE range = {};
	//CD3DX12_ROOT_PARAMETER parameter = {};
	//range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, (UINT)shader->GetConstBufferCount(), 0);
	//parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

	std::vector<CD3DX12_ROOT_PARAMETER> rootParameters;
	rootParameters.resize(shader->GetConstBufferCount());

	for (int i = 0; i < shader->GetConstBufferCount(); i++)
	{
		rootParameters[i].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	}

	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
	if (texture)
	{
		// The texture doesn't know which root parameter index it should use
		// Set it just after the CBVs
		unsigned int textureRootParamIndex = (unsigned int)rootParameters.size();
		std::dynamic_pointer_cast<DirectX12Texture2D>(texture)->SetRootParamIndex(textureRootParamIndex);

		CD3DX12_DESCRIPTOR_RANGE ranges[1] = {};
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		CD3DX12_ROOT_PARAMETER param;
		param.InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters.push_back(param);

		//std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
		staticSamplers.resize(1);

		staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		staticSamplers[0].MipLODBias = 0;
		staticSamplers[0].MaxAnisotropy = 0;
		staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		staticSamplers[0].MinLOD = 0.0f;
		staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
		staticSamplers[0].ShaderRegister = 0;
		staticSamplers[0].RegisterSpace = 0;
		staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	//rootSignatureDesc.Init(1, &parameter, 0, nullptr, rootSignatureFlags);
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

	//D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputLayout.Size()];
	// All the cool kinds use smart pointers these days insted of new/delete
	//auto inputElementDescs = std::make_unique<D3D12_INPUT_ELEMENT_DESC[]>(inputLayout.Size());
	// or just use a vector
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;

	//unsigned int inputIndex = 0;
	unsigned int offset = 0;
	for (auto& input : inputLayout)
	{
		// TODO learn what all of these do
		//SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, InstanceDataStepRate
		inputElementDescs.push_back({ input.name.c_str(), 0, GetDirectXFormat(input.type), 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		offset += input.size;
	}

	auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = RenderCommand::GetDepthTest();
	// TODO what is a stencil

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs.data(), (UINT)inputElementDescs.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = shader->GetVSBytecode();
	psoDesc.PS = shader->GetPSBytecode();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = RenderCommand::GetFaceCulling() ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FillMode = RenderCommand::GetWireframeMode() ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleMask = UINT_MAX; // sample mask has to do with multi-sampling. 0xFFFFFFFF (UINT_MAX) means point sampling is done
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1; // This must be the same as the SampleDesc for swap chain
	psoDesc.SampleDesc.Quality = 0;
	dx12context->g_pd3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)); // TODO handle fail

	// smart pointers or vectors clean themselves up
	//delete[] inputElementDescs;
}

DirectX12VertexArray::~DirectX12VertexArray()
{
}

void DirectX12VertexArray::Bind() const
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	dx12context->g_pd3dCommandList->SetPipelineState(m_pipelineState.Get());
	dx12context->g_pd3dCommandList->SetGraphicsRootSignature(m_rootSignature.Get());

	if (texture)
	{
		texture->Bind();
	}

	shader->Bind();

	vertexBuffers[0]->Bind();
	indexBuffers[0]->Bind();
}

void DirectX12VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	vertexBuffers.push_back(vertexBuffer);
}

void DirectX12VertexArray::AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
	indexBuffers.push_back(indexBuffer);
}
