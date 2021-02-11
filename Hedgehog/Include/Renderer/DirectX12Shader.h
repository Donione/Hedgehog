#pragma once

#include <Renderer/Shader.h>

#include <d3d12.h>

#include <map>
#include <string>
#include <wrl.h>


class DirectX12Shader : public Shader
{
public:
	DirectX12Shader(const std::wstring& filePath,
					const std::string& VSEntryPoint,
					const std::string& PSEntryPoint)
	{
		Create(filePath, VSEntryPoint, filePath, PSEntryPoint);
	}
	DirectX12Shader(const std::wstring& VSFilePath,
					const std::string& VSEntryPoint,
					const std::wstring& PSFilePath,
					const std::string& PSEntryPoint)
	{
		Create(VSFilePath, VSEntryPoint, PSFilePath, PSEntryPoint);
	}
	virtual ~DirectX12Shader() override;

	virtual void Bind() const override { /* do nothing */ }
	virtual void Unbind() const override { /* do nothing */ }

	virtual void SetupConstantBuffers(ConstantBufferDescription constBufferDesc) override;

	virtual void UploadConstant(const std::string& name, float constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec2 constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec3 constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec4 constant) override;

	virtual void UploadConstant(const std::string& name, glm::mat3x3 constant) override;
	virtual void UploadConstant(const std::string& name, glm::mat4x4 constant) override;

	virtual void UploadConstant(const std::string& name, int constant) override;

	virtual void UploadConstant(const std::string& name, void* constant, unsigned long long size) override;

	const D3D12_SHADER_BYTECODE GetVSBytecode() const;
	const D3D12_SHADER_BYTECODE GetPSBytecode() const;

	ID3D12DescriptorHeap* GetDescHeap() const { return CBVDescHeap; }

private:
	void Create(const std::wstring& VSFilePath,
				const std::string& VSEntryPoint,
				const std::wstring& PSFilePath,
				const std::string& PSEntryPoint);

	struct ConstantBuffer
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		UINT8* mappedData = nullptr;
		unsigned long long size = 0;
	};

	const DirectX12Shader::ConstantBuffer CreateConstantBuffer(const ConstantBufferDescriptionElement& description);

private:
	ID3DBlob* vertexShader = nullptr;
	ID3DBlob* pixelShader = nullptr;

	ConstantBufferDescription description;
	ID3D12DescriptorHeap* CBVDescHeap = nullptr;
	std::map<std::string, ConstantBuffer> constantBuffers;
};
