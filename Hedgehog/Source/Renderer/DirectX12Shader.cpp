#include <Renderer/DirectX12Shader.h>

#include <Renderer/DirectX12Context.h>
#include <Application/Application.h>

#include <d3dx12.h>
#include <d3dcompiler.h>
#include <DXSampleHelper.h>

#include <glm/gtc/type_ptr.hpp>


void DirectX12Shader::Create(const std::wstring& VSFilePath,
								 const std::string& VSEntryPoint,
								 const std::wstring& PSFilePath,
								 const std::string& PSEntryPoint)
{
	#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	#else
	UINT compileFlags = 0;
	#endif

	HRESULT isOK = D3DCompileFromFile(VSFilePath.c_str(),
									  nullptr,
									  nullptr,
									  VSEntryPoint.c_str(),
									  "vs_5_0",
									  compileFlags,
									  0,
									  &vertexShader,
									  nullptr);

	assert(SUCCEEDED(isOK));

	isOK = D3DCompileFromFile(PSFilePath.c_str(),
							  nullptr,
							  nullptr,
							  PSEntryPoint.c_str(),
							  "ps_5_0",
							  compileFlags,
							  0,
							  &pixelShader,
							  nullptr);
	assert(SUCCEEDED(isOK));
}

const DirectX12Shader::ConstantBuffer DirectX12Shader::CreateConstantBuffer(const ConstantBufferDescriptionElement& description)
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
	Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;
	UINT8* mappedData = nullptr;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(1024 * 64); // TODO any reason the example puts here 64KB?
	ThrowIfFailed(dx12context->g_pd3dDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (description.size + 255) & ~255;    // CB size is required to be 256-byte aligned.

	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(CBVDescHeap->GetCPUDescriptorHandleForHeapStart(),
											dx12context->g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
											(UINT)constantBuffers.size());
	dx12context->g_pd3dDevice->CreateConstantBufferView(&cbvDesc, cbvHandle);

	// Map and initialize the constant buffer. We don't unmap this until the
	// app closes. Keeping things mapped for the lifetime of the resource is okay.
	CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
	memset(mappedData, 0, description.size);

	return { constantBuffer, mappedData, description.size };
}

DirectX12Shader::~DirectX12Shader()
{
	if (vertexShader)
	{
		vertexShader->Release();
		vertexShader = nullptr;
	}

	if (pixelShader)
	{
		pixelShader->Release();
		pixelShader = nullptr;
	}

	if (CBVDescHeap)
	{
		CBVDescHeap->Release();
		CBVDescHeap = nullptr;
	}
}

void DirectX12Shader::SetupConstantBuffers(ConstantBufferDescription constBufferDesc)
{
	assert(constantBuffers.size() == 0);

	description = constBufferDesc;

	// Create heap descriptors for the constant buffers
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = (UINT)description.Size();
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
	dx12context->g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&CBVDescHeap));

	for (auto& element : constBufferDesc)
	{
		constantBuffers.emplace(element.name, CreateConstantBuffer(element));
	}

	// Simple way to check the constant buffer description didn't contain duplicate names
	// TODO Should be probably done in the ConstantBufferDescription constructor
	assert(constantBuffers.size() == description.Size());
}

void DirectX12Shader::UploadConstant(const std::string& name, float constant)
{
	UploadConstant(name, static_cast<void*>(&constant), sizeof(float));
}

void DirectX12Shader::UploadConstant(const std::string& name, glm::vec2 constant)
{
	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::vec2));
}

void DirectX12Shader::UploadConstant(const std::string& name, glm::vec3 constant)
{
	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::vec3));
}

void DirectX12Shader::UploadConstant(const std::string& name, glm::vec4 constant)
{
	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::vec4));
}

void DirectX12Shader::UploadConstant(const std::string& name, glm::mat3x3 constant)
{
	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::mat3x3));
}

void DirectX12Shader::UploadConstant(const std::string& name, glm::mat4x4 constant)
{

	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::mat4x4));
}

void DirectX12Shader::UploadConstant(const std::string& name, int constant)
{
	UploadConstant(name, static_cast<void*>(&constant), sizeof(int));
}

void DirectX12Shader::UploadConstant(const std::string& name, void* constant, unsigned long long size)
{
	assert(constantBuffers.contains(name));
	assert(size == constantBuffers[name].size);

	memcpy(constantBuffers[name].mappedData, constant, size);
}

const D3D12_SHADER_BYTECODE DirectX12Shader::GetVSBytecode() const
{
	if (vertexShader)
	{
		return { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	}
	else
	{
		return { nullptr, 0 };
	}
}

const D3D12_SHADER_BYTECODE DirectX12Shader::GetPSBytecode() const
{
	if (pixelShader)
	{
		return { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	}
	else
	{
		return { nullptr, 0 };
	}
}
