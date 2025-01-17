#pragma once

#include <Renderer/Shader.h>

#include <Renderer/DirectX12Context.h>

#include <d3d12.h>

#include <unordered_map>
#include <string>
#include <wrl.h>


namespace Hedge
{

class DirectX12Shader : public Shader
{
public:
	DirectX12Shader(const std::wstring& filePath,
					const std::string& VSEntryPoint,
					const std::string& PSEntryPoint,
					const std::string& GSEntryPoint = "")
	{
		Create(filePath, VSEntryPoint,
			   filePath, PSEntryPoint,
			   filePath, GSEntryPoint);
	}
	DirectX12Shader(const std::wstring& VSFilePath,
					const std::string& VSEntryPoint,
					const std::wstring& PSFilePath,
					const std::string& PSEntryPoint,
					const std::wstring& GSFilePath = std::wstring(),
					const std::string& GSEntryPoint = "")
	{
		Create(VSFilePath, VSEntryPoint,
			   PSFilePath, PSEntryPoint,
			   GSFilePath, GSEntryPoint);
	}
	virtual ~DirectX12Shader() override;

	virtual void Bind() override;
	virtual void Unbind() override;

	virtual void SetupConstantBuffers(ConstantBufferDescription constBufferDesc) override;

	virtual void UploadConstant(const std::string& name, float constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec2 constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec3 constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec4 constant) override;

	virtual void UploadConstant(const std::string& name, glm::mat3x3 constant) override;
	virtual void UploadConstant(const std::string& name, glm::mat4x4 constant) override;

	virtual void UploadConstant(const std::string& name, const std::vector<glm::mat4>& constant) override;

	virtual void UploadConstant(const std::string& name, int constant) override;

	virtual void UploadConstant(const std::string& name, const DirectionalLight& constant) override;
	virtual void UploadConstant(const std::string& name, const PointLight& constant) override;
	virtual void UploadConstant(const std::string& name, const SpotLight& constant) override;

	virtual void UploadConstant(const std::string& name, const DirectionalLight* constant, int count = 1) override;
	virtual void UploadConstant(const std::string& name, const PointLight* constant, int count = 1) override;
	virtual void UploadConstant(const std::string& name, const SpotLight* constant, int count = 1) override;

	virtual void UploadConstant(const std::string& name, const void* constant, unsigned long long size) override;

	virtual const size_t GetConstBufferCount() const override { return constantBuffers.size(); }

	const D3D12_SHADER_BYTECODE GetVSBytecode() const;
	const D3D12_SHADER_BYTECODE GetPSBytecode() const;
	const D3D12_SHADER_BYTECODE GetGSBytecode() const;

private:
	void Create(const std::wstring& VSFilePath,
				const std::string& VSEntryPoint,
				const std::wstring& PSFilePath,
				const std::string& PSEntryPoint,
				const std::wstring& GSFilePath = std::wstring(),
				const std::string& GSEntryPoint = "");

	bool CheckCompileError(HRESULT result, ID3DBlob* pErrorBlob);

	struct ConstantBufferView
	{
		UINT8* mappedData[DirectX12Context::NUM_FRAMES_IN_FLIGHT] = {};
		unsigned long long size = 0;
		unsigned long long totalSize = 0;
	};

	struct ConstantBuffer
	{
		unsigned int rootParamIndex = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer[DirectX12Context::NUM_FRAMES_IN_FLIGHT];
		UINT8* mappedData[DirectX12Context::NUM_FRAMES_IN_FLIGHT] = {};
		unsigned long long totalSize = 0;
	};

	DirectX12Shader::ConstantBuffer CreateConstantBuffer(ConstantBufferUsage usage);
	unsigned long long GetConstantBufferSize(ConstantBufferUsage usage);
	DirectX12Shader::ConstantBufferView CreateConstantBufferView(ConstantBufferDescriptionElement element);


private:
	ID3DBlob* vertexShader = nullptr;
	ID3DBlob* pixelShader = nullptr;
	ID3DBlob* geometryShader = nullptr;

	ConstantBufferDescription description;
	//ID3D12DescriptorHeap* CBVDescHeap = nullptr;
	std::unordered_map<ConstantBufferUsage, ConstantBuffer> constantBuffers;
	int constBuffersSkipped = 0;
	std::unordered_map<std::string, ConstantBufferView> constantBufferViews;
	std::unordered_map <ConstantBufferUsage, unsigned long long> dataOffsets =
	{
		{ ConstantBufferUsage::Scene, 0 },
		{ ConstantBufferUsage::Light, 0 },
		{ ConstantBufferUsage::Object, 0 },
		{ ConstantBufferUsage::Other, 0 }
	};

	long long int objectNum = 0;
};

} // namespace Hedge
