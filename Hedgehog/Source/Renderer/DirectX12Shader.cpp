#include <Renderer/DirectX12Shader.h>

#include <Application/Application.h>

#include <d3dx12.h>
#include <d3dcompiler.h>
#include <DXSampleHelper.h>

#include <glm/gtc/type_ptr.hpp>


namespace Hedge
{

void DirectX12Shader::Create(const std::wstring& VSFilePath,
							 const std::string& VSEntryPoint,
							 const std::wstring& PSFilePath,
							 const std::string& PSEntryPoint,
							 const std::wstring& GSFilePath,
							 const std::string& GSEntryPoint)
{
	#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	#else
	UINT compileFlags = 0;
	#endif

	bool useGeometryShader = !GSFilePath.empty() && !GSEntryPoint.empty();

	ID3DBlob* pErrorBlob;
	HRESULT result = D3DCompileFromFile(VSFilePath.c_str(),
										nullptr,
										nullptr,
										VSEntryPoint.c_str(),
										"vs_5_0",
										compileFlags,
										0,
										&vertexShader,
										&pErrorBlob);
	assert(CheckCompileError(result, pErrorBlob));

	result = D3DCompileFromFile(PSFilePath.c_str(),
								nullptr,
								nullptr,
								PSEntryPoint.c_str(),
								"ps_5_0",
								compileFlags,
								0,
								&pixelShader,
								&pErrorBlob);
	assert(CheckCompileError(result, pErrorBlob));

	if (useGeometryShader)
	{
		result = D3DCompileFromFile(GSFilePath.c_str(),
									nullptr,
									nullptr,
									GSEntryPoint.c_str(),
									"gs_5_0",
									compileFlags,
									0,
									&geometryShader,
									&pErrorBlob);
		assert(CheckCompileError(result, pErrorBlob));
	}
}

bool DirectX12Shader::CheckCompileError(HRESULT result, ID3DBlob* pErrorBlob)
{
	if (pErrorBlob != nullptr)
	{
		printf("%s\n", (char*)pErrorBlob->GetBufferPointer());
		pErrorBlob->Release();
	}
	return SUCCEEDED(result);
}

DirectX12Shader::ConstantBuffer DirectX12Shader::CreateConstantBuffer(ConstantBufferUsage usage)
{
	unsigned int rootParamIndex = 0;
	switch (usage)
	{
	case ConstantBufferUsage::Scene: rootParamIndex = 0; break;
	case ConstantBufferUsage::Light: rootParamIndex = 1; break;
	case ConstantBufferUsage::Object: rootParamIndex = 2; break;
	case ConstantBufferUsage::Other: rootParamIndex = 3; break;
	default: assert(false); break;
	}

	unsigned long long size = GetConstantBufferSize(usage);
	if (size == 0)
	{
		constBuffersSkipped++;
		return {};
	}

	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
	ConstantBuffer constantBuffer = { rootParamIndex - constBuffersSkipped, {}, {}, size };

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(1024 * 64); // TODO any reason the example puts here 64KB? Seems that buffers need to be 64KB aligned
	for (int i = 0; i < DirectX12Context::NUM_FRAMES_IN_FLIGHT; i++)
	{
		ThrowIfFailed(dx12context->g_pd3dDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constantBuffer.buffer[i])));

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(constantBuffer.buffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&constantBuffer.mappedData[i])));
		// Any reason to zero the newly created constant buffer?
	}

	return constantBuffer;
}

unsigned long long DirectX12Shader::GetConstantBufferSize(ConstantBufferUsage usage)
{
	unsigned long long size = 0;
	for (auto& element : description)
	{
		if (element.usage == usage)
		{
			// TODO properly follow packing rules
			// the following won't work on floats and float2s
			size += ((element.size * element.count + 15) & ~15);
		}
	}
	// CB size is required to be 256-byte aligned.
	return (size + 255) & ~255;
}

DirectX12Shader::ConstantBufferView DirectX12Shader::CreateConstantBufferView(ConstantBufferDescriptionElement element)
{
	ConstantBufferView newBuffer;
	for (int i = 0; i < DirectX12Context::NUM_FRAMES_IN_FLIGHT; i++)
	{
		newBuffer.mappedData[i] = constantBuffers[element.usage].mappedData[i] + dataOffsets[element.usage];
	}
	newBuffer.size = element.size * element.count;
	newBuffer.totalSize = constantBuffers[element.usage].totalSize;

	dataOffsets.at(element.usage) += ((element.size * element.count + 15) & ~15);

	return newBuffer;
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

	//if (CBVDescHeap)
	//{
	//	CBVDescHeap->Release();
	//	CBVDescHeap = nullptr;
	//}
	
	// TODO probably should unmap constant buffers
}

void DirectX12Shader::Bind()
{
	DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
	int frameIndex = (dx12context->g_frameIndex % dx12context->NUM_FRAMES_IN_FLIGHT);

	//dx12context->g_pd3dCommandList->SetDescriptorHeaps(1, &CBVDescHeap);
	//// Find the correct offset into the descriptor heap
	//CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(CBVDescHeap->GetGPUDescriptorHandleForHeapStart(),
	//										frameIndex * 4 + bindCount * 2,
	//										dx12context->g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	//// The pipeline will use the number of descriptors from the (offseted) heap acording to the root signature
	//dx12context->g_pd3dCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

	// TODO assert we're not going outside of allocated buffer
	for (auto const& [key, constBuffer] : constantBuffers)
	{
		// Scene constant buffers are (should) be updated once per frame at most, so no need to offset
		// TODO We are assuming here that a different pipeline HASN'T been bound in between multiple bind calls to the vertex array associated with this shader
		if (key == ConstantBufferUsage::Scene)
		{
			dx12context->g_pd3dCommandList->SetGraphicsRootConstantBufferView(constBuffer.rootParamIndex,
																			  constBuffer.buffer[frameIndex]->GetGPUVirtualAddress());
		}
		else
		{
			dx12context->g_pd3dCommandList->SetGraphicsRootConstantBufferView(constBuffer.rootParamIndex,
																			  constBuffer.buffer[frameIndex]->GetGPUVirtualAddress() + objectNum * constBuffer.totalSize);
		}
	}

	objectNum++;
}

void DirectX12Shader::Unbind()
{
	objectNum = 0;
}

void DirectX12Shader::SetupConstantBuffers(ConstantBufferDescription constBufferDesc)
{
	// Setup should be called only once
	assert(constantBuffers.size() == 0);

	description = constBufferDesc;

	//DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());

	//// Create heap descriptors for the constant buffers
	//D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	//desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//desc.NumDescriptors = (UINT)description.Size() * dx12context->NUM_FRAMES_IN_FLIGHT * numObjects;
	//desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//dx12context->g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&CBVDescHeap));

	std::vector<ConstantBufferUsage> usages = { ConstantBufferUsage::Scene, ConstantBufferUsage::Light, ConstantBufferUsage::Object, ConstantBufferUsage::Other };
	for (auto usage : usages)
	{
		ConstantBuffer newBuffer = CreateConstantBuffer(usage);
		if (newBuffer.totalSize != 0) constantBuffers.emplace(usage, newBuffer);
	}

	for (auto& element : description)
	{
		constantBufferViews.emplace(element.name, CreateConstantBufferView(element));
	}

	// Simple way to check the constant buffer description didn't contain duplicate names
	// TODO Should be probably done in the ConstantBufferDescription constructor
	assert(constantBufferViews.size() == description.Size());
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

void DirectX12Shader::UploadConstant(const std::string& name, const std::vector<glm::mat4>& constant)
{
	UploadConstant(name, static_cast<const void*>(constant.data()), sizeof(glm::mat4) * constant.size());
}

void DirectX12Shader::UploadConstant(const std::string& name, int constant)
{
	UploadConstant(name, static_cast<void*>(&constant), sizeof(int));
}

void DirectX12Shader::UploadConstant(const std::string& name, const DirectionalLight& constant)
{
	UploadConstant(name, static_cast<const void*>(&constant), sizeof(DirectionalLight));
}

void DirectX12Shader::UploadConstant(const std::string& name, const PointLight& constant)
{
	UploadConstant(name, static_cast<const void*>(&constant), sizeof(PointLight));
}

void DirectX12Shader::UploadConstant(const std::string& name, const SpotLight& constant)
{
	UploadConstant(name, static_cast<const void*>(&constant), sizeof(SpotLight));
}

void DirectX12Shader::UploadConstant(const std::string& name, const DirectionalLight* constant, int count)
{
	UploadConstant(name, static_cast<const void*>(constant), sizeof(DirectionalLight) * count);
}

void DirectX12Shader::UploadConstant(const std::string& name, const PointLight* constant, int count)
{
	UploadConstant(name, static_cast<const void*>(constant), sizeof(PointLight) * count);
}

void DirectX12Shader::UploadConstant(const std::string& name, const SpotLight* constant, int count)
{
	UploadConstant(name, static_cast<const void*>(constant), sizeof(SpotLight) * count);
}

void DirectX12Shader::UploadConstant(const std::string& name, const void* constant, unsigned long long size)
{
	if (constantBufferViews.contains(name))
	{
		assert(size <= constantBufferViews.at(name).size);

		DirectX12Context* dx12context = dynamic_cast<DirectX12Context*>(Application::GetInstance().GetRenderContext());
		int frameIndex = (dx12context->g_frameIndex % dx12context->NUM_FRAMES_IN_FLIGHT);

		// TODO assert no overflow
		memcpy(constantBufferViews.at(name).mappedData[frameIndex] + objectNum * constantBufferViews.at(name).totalSize, constant, size);
	}
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

const D3D12_SHADER_BYTECODE DirectX12Shader::GetGSBytecode() const
{
	if (geometryShader)
	{
		return { geometryShader->GetBufferPointer(), geometryShader->GetBufferSize() };
	}
	else
	{
		return { nullptr, 0 };
	}
}

} // namespace Hedge
