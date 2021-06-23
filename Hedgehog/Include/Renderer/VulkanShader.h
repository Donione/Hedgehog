#pragma once

#include <Renderer/Shader.h>

#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <stdint.h>
#include <memory>
#include <unordered_map>

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

	virtual void Bind() override;
	virtual void Unbind() override;

	virtual void SetupConstantBuffers(ConstantBufferDescription constBufferDesc) override;

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

	virtual void UploadConstant(const std::string& name, const void* constant, unsigned long long size) override;

	virtual const size_t GetConstBufferCount() const override { return uniformBuffers.size(); }


	const VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }
	const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() const { return shaderStages; }

private:
	const std::unique_ptr<uint32_t> ReadFile(const std::string& filePath, size_t* size);
	VkShaderModule CreateShaderModule(const std::string& filePath);
	VkPipelineShaderStageCreateInfo CreateShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule) const;

	struct UniformBuffer
	{
		size_t dataSize;
		size_t bufferSize;
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> mappedMemory;
	};

	struct UniformVariableView
	{
		ConstantBufferUsage set;
		size_t offset;
		size_t size;
	};

	UniformBuffer CreateUniformBuffer(ConstantBufferUsage set) const;
	size_t GetUniformBufferSize(ConstantBufferUsage set) const;
	UniformVariableView CreateUniformVariableView(ConstantBufferDescriptionElement element);
	void CreateDescriptorSetLayouts();
	VkDescriptorSetLayout CreateDescriptorSetLayout(unsigned int numberOfBindings) const;
	std::vector<std::vector<VkDescriptorSet>> CreateDescriptorSets() const;
	VkPipelineLayout CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) const;


private:
	VkShaderModule vertexShaderModule = VK_NULL_HANDLE;
	VkShaderModule pixelShaderModule = VK_NULL_HANDLE;
	VkShaderModule geometryShaderModule = VK_NULL_HANDLE;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	ConstantBufferDescription uniformBufferDescription;
	std::unordered_map<ConstantBufferUsage, UniformBuffer> uniformBuffers;
	std::unordered_map<std::string, UniformVariableView> uniformVariableViews;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::vector<std::vector<std::vector<VkDescriptorSet>>> descriptorSets;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	std::unordered_map <ConstantBufferUsage, size_t> elementOffsets =
	{
		{ ConstantBufferUsage::Scene, 0 },
		{ ConstantBufferUsage::Light, 0 },
		{ ConstantBufferUsage::Object, 0 },
		{ ConstantBufferUsage::Other, 0 }
	};

	size_t bindCount = 0;
};

} // namespace Hedge
