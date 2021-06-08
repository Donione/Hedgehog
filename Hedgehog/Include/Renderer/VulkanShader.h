#pragma once

#include <Renderer/Shader.h>

#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <stdint.h>
#include <memory>

#include <vulkan/vulkan.h>


namespace Hedge
{

class VulkanShader : public Shader
{
public:
	VulkanShader(const std::string& VSFilePath,
				 const std::string& PSFilePath,
				 const std::string& GSFilePath = "");
	virtual ~VulkanShader() override;

	virtual void Bind() override {}
	virtual void Unbind() override {}

	virtual void SetupConstantBuffers(ConstantBufferDescription constBufferDesc) override {}

	virtual void UploadConstant(const std::string& name, float constant) override {}
	virtual void UploadConstant(const std::string& name, glm::vec2 constant) override {}
	virtual void UploadConstant(const std::string& name, glm::vec3 constant) override {}
	virtual void UploadConstant(const std::string& name, glm::vec4 constant) override {}

	virtual void UploadConstant(const std::string& name, glm::mat3x3 constant) override {}
	virtual void UploadConstant(const std::string& name, glm::mat4x4 constant) override {}

	virtual void UploadConstant(const std::string& name, const std::vector<glm::mat4>& constant) override {}

	virtual void UploadConstant(const std::string& name, int constant) override {}

	virtual void UploadConstant(const std::string& name, const DirectionalLight& constant) override {}
	virtual void UploadConstant(const std::string& name, const PointLight& constant) override {}
	virtual void UploadConstant(const std::string& name, const SpotLight& constant) override {}

	virtual void UploadConstant(const std::string& name, const DirectionalLight* constant, int count = 1) override {}
	virtual void UploadConstant(const std::string& name, const PointLight* constant, int count = 1) override {}
	virtual void UploadConstant(const std::string& name, const SpotLight* constant, int count = 1) override {}

	virtual void UploadConstant(const std::string& name, const void* constant, unsigned long long size) override {}

	const std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages() const { return shaderStages; }

private:
	const std::unique_ptr<uint32_t> ReadFile(const std::string& filePath, size_t* size);
	VkShaderModule CreateShaderModule(const std::string& filePath);
	VkPipelineShaderStageCreateInfo CreateShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule);


private:
	VkShaderModule vertexShaderModule = VK_NULL_HANDLE;
	VkShaderModule pixelShaderModule = VK_NULL_HANDLE;
	VkShaderModule geometryShaderModule = VK_NULL_HANDLE;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
};

} // namespace Hedge
