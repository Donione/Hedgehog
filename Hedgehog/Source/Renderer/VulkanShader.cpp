#include <Renderer/VulkanShader.h>

#include <Application/Application.h>
#include <Renderer/VulkanContext.h>

#include <fstream>


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

VkPipelineShaderStageCreateInfo VulkanShader::CreateShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule)
{
	VkPipelineShaderStageCreateInfo shaderStage{};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = shaderModule;
	shaderStage.pName = "main";

	return shaderStage;
}

} // namespace Hedge
