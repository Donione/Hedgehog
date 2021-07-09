#include <Renderer/VulkanShader.h>

#include <Application/Application.h>
#include <Renderer/VulkanContext.h>

#include <fstream>

#include <glm/gtc/type_ptr.hpp>


namespace Hedge
{

Hedge::VulkanShader::VulkanShader(const std::string& VSFilePath,
								  const std::string& PSFilePath,
								  const std::string& GSFilePath)
{
	bool useGeometryShader = !GSFilePath.empty();

	vertexShaderModule = CreateShaderModule(VSFilePath);
	shaderStages.push_back(CreateShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShaderModule));

	pixelShaderModule = CreateShaderModule(PSFilePath);
	shaderStages.push_back(CreateShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, pixelShaderModule));

	if (useGeometryShader)
	{
		geometryShaderModule = CreateShaderModule(GSFilePath);
		shaderStages.push_back(CreateShaderStage(VK_SHADER_STAGE_GEOMETRY_BIT, geometryShaderModule));
	}
}

VulkanShader::~VulkanShader()
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	vkDestroyShaderModule(vulkanContext->device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(vulkanContext->device, pixelShaderModule, nullptr);
	vkDestroyShaderModule(vulkanContext->device, geometryShaderModule, nullptr);

	for (auto descriptorSetLayout : descriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(vulkanContext->device, descriptorSetLayout, nullptr);
	}
	vkDestroyPipelineLayout(vulkanContext->device, pipelineLayout, nullptr);

	for (auto& [usage, constantBuffer] : uniformBuffers)
	{
		for (int i = 0; i < constantBuffer.uniformBuffers.size(); i++)
		{
			vkUnmapMemory(vulkanContext->device, constantBuffer.uniformBuffersMemory[i]);
			vulkanContext->DestoyVulkanBuffer(constantBuffer.uniformBuffers[i], constantBuffer.uniformBuffersMemory[i]);
		}
		constantBuffer.uniformBuffers.clear();
		constantBuffer.uniformBuffersMemory.clear();
	}
}

void VulkanShader::Bind()
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	// For each draw call using the same pipeline within one frame we need a separate desriptor sets
	// which point to the same uniform buffer but with per-call offsets
	// TODO Other way to do this would be to use dynamic uniform buffers
	// We create the descriptors once and keep them alive until the application closes
	// We could also allocate and create them each frame on demand, which should be cheap if our descriptor pools
	// have freeing of individual sets disabled (this is controled by the VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT flag)
	if (bindCount + 1 > descriptorSets.size())
	{
		descriptorSets.push_back(CreateDescriptorSets());
	}

	std::vector<VkDescriptorSet> currentDescriptorSets = descriptorSets[bindCount][vulkanContext->swapChainImageIndex];

	uint32_t firstSet = 0;
	uint32_t descriptorSetCount = static_cast<uint32_t>(currentDescriptorSets.size());
	VkDescriptorSet* pDescriptorSets = currentDescriptorSets.data();
	// If there is a descriptor set for scene data, it is enough to bind it only once per frame
	// so for subsequent bind call we skip binding the scene data descriptor set
	// TODO We are assuming here that an incomatible pipeline HASN'T been bound in between bind calls to the vertex array associated with this shader
	if (bindCount > 0
		&& uniformBuffers.contains(ConstantBufferUsage::Scene))
	{
		// Descriptor set for scene data is always first in the current implementation
		firstSet = 1;
		descriptorSetCount--;
		pDescriptorSets = &currentDescriptorSets.data()[1];
	}

	if (descriptorSetCount > 0)
	{
		vkCmdBindDescriptorSets(vulkanContext->commandBuffers[vulkanContext->swapChainImageIndex],
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipelineLayout,
								firstSet,
								descriptorSetCount,
								pDescriptorSets,
								0,
								nullptr);
	}

	bindCount++;
}

void VulkanShader::Unbind()
{
	bindCount = 0;
}

void VulkanShader::SetupConstantBuffers(ConstantBufferDescription constBufferDesc)
{
	uniformBufferDescription = constBufferDesc;

	std::vector<ConstantBufferUsage> sets =
	{
		ConstantBufferUsage::Scene,
		ConstantBufferUsage::Light,
		ConstantBufferUsage::Object,
		ConstantBufferUsage::Other
	};

	for (auto set : sets)
	{
		UniformBuffer uniformBuffer = CreateUniformBuffer(set);
		if (uniformBuffer.dataSize != 0) uniformBuffers.emplace(set, uniformBuffer);
	}

	for (auto& element : uniformBufferDescription)
	{
		uniformVariableViews.emplace(element.name, CreateUniformVariableView(element));
	}

	CreateDescriptorSetLayouts();
	pipelineLayout = CreatePipelineLayout(descriptorSetLayouts);
	descriptorSets.push_back(CreateDescriptorSets());
}

void VulkanShader::UploadConstant(const std::string& name, float constant)
{
	UploadConstant(name, static_cast<void*>(&constant), sizeof(float));
}

void VulkanShader::UploadConstant(const std::string& name, glm::vec2 constant)
{
	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::vec2));
}

void VulkanShader::UploadConstant(const std::string& name, glm::vec3 constant)
{
	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::vec3));
}

void VulkanShader::UploadConstant(const std::string& name, glm::vec4 constant)
{
	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::vec4));
}

void VulkanShader::UploadConstant(const std::string& name, glm::mat3x3 constant)
{
	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::mat3x3));
}

void VulkanShader::UploadConstant(const std::string& name, glm::mat4x4 constant)
{
	UploadConstant(name, static_cast<void*>(glm::value_ptr(constant)), sizeof(glm::mat4x4));
}

void VulkanShader::UploadConstant(const std::string& name, const std::vector<glm::mat4>& constant)
{
	UploadConstant(name, static_cast<const void*>(constant.data()), sizeof(glm::mat4) * constant.size());
}

void VulkanShader::UploadConstant(const std::string& name, int constant)
{
	UploadConstant(name, static_cast<void*>(&constant), sizeof(int));
}

void VulkanShader::UploadConstant(const std::string& name, const DirectionalLight& constant)
{
	UploadConstant(name, static_cast<const void*>(&constant), sizeof(DirectionalLight));
}

void VulkanShader::UploadConstant(const std::string& name, const PointLight& constant)
{
	UploadConstant(name, static_cast<const void*>(&constant), sizeof(PointLight));
}

void VulkanShader::UploadConstant(const std::string& name, const SpotLight& constant)
{
	UploadConstant(name, static_cast<const void*>(&constant), sizeof(SpotLight));
}

void VulkanShader::UploadConstant(const std::string& name, const DirectionalLight* constant, int count)
{
	UploadConstant(name, static_cast<const void*>(constant), sizeof(DirectionalLight) * count);
}

void VulkanShader::UploadConstant(const std::string& name, const PointLight* constant, int count)
{
	UploadConstant(name, static_cast<const void*>(constant), sizeof(PointLight) * count);
}

void VulkanShader::UploadConstant(const std::string& name, const SpotLight* constant, int count)
{
	UploadConstant(name, static_cast<const void*>(constant), sizeof(SpotLight) * count);
}

void VulkanShader::UploadConstant(const std::string& name, const void* constant, unsigned long long size)
{
	if (!uniformVariableViews.contains(name)) return;

	UniformVariableView& uniformVariable = uniformVariableViews.at(name);

	assert(size <= uniformVariable.size);

	auto vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	void* mappedMemmory = uniformBuffers.at(uniformVariable.set).mappedMemory[vulkanContext->swapChainImageIndex];
	size_t dataSize = uniformBuffers.at(uniformVariable.set).dataSize;
	// TODO get rid of the hard-coded alignment calculations
	memcpy(static_cast<uint8_t*>(mappedMemmory) + bindCount * ((dataSize + 0x3F) & ~0x3F) + uniformVariable.offset,
		   constant,
		   size);
}

const std::unique_ptr<uint32_t> VulkanShader::ReadFile(const std::string& filePath, size_t* size)
{
	uint32_t* data = nullptr;

	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	assert(file.is_open());

	size_t fileSize = file.tellg();
	// TODO are spv files 4-byte aligned out of the compiler?
	assert(fileSize % 4 == 0);
	size_t dataSize = fileSize / 4;
	data = new uint32_t[dataSize];

	file.seekg(0);
	file.read(reinterpret_cast<char*>(data), fileSize);

	file.close();

	*size = fileSize;
	return std::unique_ptr<uint32_t>(data);
}

VkShaderModule VulkanShader::CreateShaderModule(const std::string& filePath)
{
	VkShaderModule shaderModule = VK_NULL_HANDLE;
	size_t size = 0;
	std::unique_ptr<uint32_t> shaderByteCode = ReadFile(filePath, &size);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = shaderByteCode.get();

	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());
	if (vkCreateShaderModule(vulkanContext->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		assert(false);
	}

	return shaderModule;
}

VkPipelineShaderStageCreateInfo VulkanShader::CreateShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule) const
{
	VkPipelineShaderStageCreateInfo shaderStage{};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = shaderModule;
	shaderStage.pName = "main";

	return shaderStage;
}

// TODO For now we have one uniform buffer (binding) per descriptor set
// so practically descriptor set == uniform buffer
VulkanShader::UniformBuffer VulkanShader::CreateUniformBuffer(ConstantBufferUsage set) const
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	UniformBuffer constantBuffer;

	constantBuffer.dataSize = GetUniformBufferSize(set);
	if (constantBuffer.dataSize == 0)
	{
		return constantBuffer;
	}

	constantBuffer.bufferSize = 64 * 1024; // TODO this is just an arbitrary number, it is also used in DX12 constant buffers because allignment rules
	constantBuffer.uniformBuffers.resize(vulkanContext->swapchainImages.size());
	constantBuffer.uniformBuffersMemory.resize(vulkanContext->swapchainImages.size());
	constantBuffer.mappedMemory.resize(vulkanContext->swapchainImages.size());

	for (int i = 0; i < vulkanContext->swapchainImages.size(); i++)
	{
		vulkanContext->CreateBuffer(constantBuffer.bufferSize,
									VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
									&constantBuffer.uniformBuffers[i],
									&constantBuffer.uniformBuffersMemory[i]);

		// Map the constant buffer here and keep it mapped throught the life of the constant buffer
		VkResult result = vkMapMemory(vulkanContext->device,
									  constantBuffer.uniformBuffersMemory[i],
									  0,
									  constantBuffer.bufferSize,
									  0,
									  &constantBuffer.mappedMemory[i]);
		if (result != VK_SUCCESS)
		{
			assert(false);
		}
	}

	return constantBuffer;
}

size_t VulkanShader::GetUniformBufferSize(ConstantBufferUsage set) const
{
	size_t size = 0;

	for (auto& element : uniformBufferDescription)
	{
		if (element.usage == set)
		{
			// TODO properly follow packing rules
			size += ((element.size * element.count + 15) & ~15);
		}
	}

	return size;
}

VulkanShader::UniformVariableView VulkanShader::CreateUniformVariableView(ConstantBufferDescriptionElement element)
{
	UniformVariableView uniformVariableView{};

	uniformVariableView.set = element.usage;
	uniformVariableView.offset = elementOffsets[element.usage];
	uniformVariableView.size = element.size * element.count;

	elementOffsets[element.usage] += ((uniformVariableView.size + 15) & ~15);

	return uniformVariableView;
}

void VulkanShader::CreateDescriptorSetLayouts()
{
	for (int set = 0; set < uniformBuffers.size(); set++)
	{
		descriptorSetLayouts.push_back(CreateDescriptorSetLayout(1)); // GetNumberOfBindings(set);
	}
}

VkDescriptorSetLayout VulkanShader::CreateDescriptorSetLayout(unsigned int numberOfBindings) const
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
	for (uint32_t binding = 0; binding < numberOfBindings; binding++)
	{
		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
		descriptorSetLayoutBinding.binding = binding;
		descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetLayoutBinding.descriptorCount = 1; // TODO arrays of uniform buffers
		descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL; // TODO add to API
		descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

		descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
	}

	descriptorSetLayoutCreateInfo.bindingCount = numberOfBindings;
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkResult result = vkCreateDescriptorSetLayout(vulkanContext->device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		assert(false);
	}

	return descriptorSetLayout;
}

std::vector<std::vector<VkDescriptorSet>> VulkanShader::CreateDescriptorSets() const
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	std::vector<std::vector<VkDescriptorSet>> descriptorSets;
	descriptorSets.resize(vulkanContext->swapchainImages.size());
	size_t objectNumber = this->descriptorSets.size();

	for (int i = 0; i < vulkanContext->swapchainImages.size(); i++)
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = vulkanContext->descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		descriptorSets[i].resize(descriptorSetLayouts.size());
		if (vkAllocateDescriptorSets(vulkanContext->device, &descriptorSetAllocateInfo, descriptorSets[i].data()) != VK_SUCCESS)
		{
			assert(false);
		}

		int setIndex = 0;
		for (auto& [set, uniformBuffer] : uniformBuffers)
		{
			VkDescriptorBufferInfo descriptorBufferInfo{};
			descriptorBufferInfo.buffer = uniformBuffer.uniformBuffers[i];
			// TODO We should poll the hardware to get the alignment requirement value
			descriptorBufferInfo.offset = objectNumber * ((uniformBuffer.dataSize + 0x3F) & ~0x3F); // 0x40 alignment requirement for offsets;
			// Make sure we're not trying to use more space that we allocated for the uniform buffer
			// Note that right now we're creating the uniform buffers with a fixed size of 64KB
			assert(descriptorBufferInfo.offset + descriptorBufferInfo.range <= uniformBuffer.bufferSize);
			descriptorBufferInfo.range = uniformBuffer.dataSize;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i][setIndex++];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &descriptorBufferInfo;

			// We could update all descriptorSets with one call to vkUpdateDescriptorSets
			// but for that we need to hold all of the VkDescriptorBufferInfo and VkWriteDescriptorSet structures for a while
			// and that's just too much hassle just for less calls
			vkUpdateDescriptorSets(vulkanContext->device,
									1,
									&descriptorWrite,
									0,
									nullptr);
		}
	}

	return descriptorSets;
}

VkPipelineLayout VulkanShader::CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) const
{
	VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();;

	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	if (vkCreatePipelineLayout(vulkanContext->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		assert(false);
	}

	return pipelineLayout;
}

} // namespace Hedge
