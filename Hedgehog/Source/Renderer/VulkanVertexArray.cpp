#include <Renderer/VulkanVertexArray.h>

#include <Application/Application.h>
#include <Renderer/VulkanRendererAPI.h>

#include <vulkan/vulkan.h>


namespace Hedge
{

VulkanVertexArray::VulkanVertexArray(const std::shared_ptr<Shader>& inputShader,
									 PrimitiveTopology primitiveTopology,
									 const BufferLayout& inputLayout,
									 const std::vector<Hedge::TextureDescription>& textureDescriptions)
	:
	baseShader(inputShader),
	shader(std::dynamic_pointer_cast<VulkanShader>(inputShader)),
	primitiveTopology(primitiveTopology),
	bufferLayout(inputLayout),
	textureDescriptions(textureDescriptions),
	inputAssemblyState(CreateInputAssemblyState(primitiveTopology)),
	viewportState(CreateViewportState()),
	rasterizationState(CreateRasterizationState()),
	multisampleState(CreateMultisampleState()),
	depthStencilState(CreateDepthStencilState()),
	colorBlendAttachmentState(CreateColorBlendAttachmentState()),
	colorBlendState(CreateColorBlendState()),
	dynamicState(CreatDynamicState())
{
	vulkanContext = dynamic_cast<VulkanContext*>(Application::GetInstance().GetRenderContext());

	const VulkanRendererAPI* renderer = dynamic_cast<const VulkanRendererAPI*>(RenderCommand::GetRenderer());
	viewport = renderer->GetViewport();
	scissor = renderer->GetScissor();

	pipelineLayout = CreatePipelineLayout();
}

VulkanVertexArray::~VulkanVertexArray()
{
	vkDestroyPipelineLayout(vulkanContext->device, pipelineLayout, nullptr);
	vkDestroyPipeline(vulkanContext->device, pipeline, nullptr);
}

void VulkanVertexArray::Bind()
{
	if (pipeline == VK_NULL_HANDLE)
	{
		vertexInputState = CreateVertexInputState();
		CreatePipeline();
	}

	vkCmdBindPipeline(vulkanContext->commandBuffers[vulkanContext->frameInFlightIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	unsigned int slot = 0;
	for (auto& vertexBuffer : vertexBuffers)
	{
		vertexBuffer->Bind(slot++);
	}

	indexBuffer->Bind();
}

void VulkanVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	vertexBuffers.push_back(vertexBuffer);
	bufferLayout += vertexBuffer->GetLayout();
	strides.push_back(vertexBuffer->GetLayout().GetStride());
}

void VulkanVertexArray::AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
	this->indexBuffer = indexBuffer;
}

void VulkanVertexArray::AddTexture(TextureType type, int position, const std::shared_ptr<Texture>& texture)
{
}

void VulkanVertexArray::SetupGroups(const std::vector<VertexGroup>& groups)
{
	for (auto& group : groups)
	{
		this->groups.emplace_back(group, 0.0f);
	}
}

void VulkanVertexArray::Resize(int width, int height)
{
	vkDestroyPipeline(vulkanContext->device, pipeline, nullptr);

	const VulkanRendererAPI* renderer = dynamic_cast<const VulkanRendererAPI*>(RenderCommand::GetRenderer());
	viewport = renderer->GetViewport();
	scissor = renderer->GetScissor();

	CreatePipeline();
}

void VulkanVertexArray::CreatePipeline()
{
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	pipelineInfo.stageCount = static_cast<uint32_t>(shader->GetShaderStages().size());
	pipelineInfo.pStages = shader->GetShaderStages().data();
	pipelineInfo.pVertexInputState = &vertexInputState;
	pipelineInfo.pInputAssemblyState = &inputAssemblyState;
	//pipelineInfo.pTessellationState = &tessellationState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pDepthStencilState = nullptr; // &depthStencilState; // TODO skip depth testing for now
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.pDynamicState = nullptr; // &dynamicState; // TODO skip dynamic state for now
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = vulkanContext->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(vulkanContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		assert(false);
	}
}

VkPipelineVertexInputStateCreateInfo VulkanVertexArray::CreateVertexInputState()
{
	int currentBinding = -1;
	for (auto& input : bufferLayout)
	{
		if (input.inputSlot != currentBinding)
		{
			VkVertexInputBindingDescription vertexBindingDescription{};
			vertexBindingDescription.binding = input.inputSlot;
			vertexBindingDescription.stride = strides[input.inputSlot];
			if (input.instanceDataStep == 0)
			{
				vertexBindingDescription.inputRate =  VK_VERTEX_INPUT_RATE_VERTEX;
			}
			else
			{
				vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

				VkVertexInputBindingDivisorDescriptionEXT vertexBindingDivisorDescription{};
				vertexBindingDivisorDescription.binding = input.inputSlot;
				vertexBindingDivisorDescription.divisor = input.instanceDataStep;

				vertexBindingDivisorDescriptions.push_back(vertexBindingDivisorDescription);
			}

			vertexBindingDescriptions.push_back(vertexBindingDescription);

			currentBinding = input.inputSlot;
		}

		VkVertexInputAttributeDescription vertexAttributeDescription{};
		vertexAttributeDescription.location = vertexAttributeLocation;
		vertexAttributeDescription.binding = currentBinding;
		vertexAttributeDescription.format = GetVulkanFormat(input.type);
		vertexAttributeDescription.offset = static_cast<uint32_t>(input.offset);

		vertexAttributeDescriptions.push_back(vertexAttributeDescription);

		vertexAttributeLocation++;
	}

	vertexInputDivisiorState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
	vertexInputDivisiorState.pNext = nullptr;
	vertexInputDivisiorState.vertexBindingDivisorCount = static_cast<uint32_t>(vertexBindingDivisorDescriptions.size());
	vertexInputDivisiorState.pVertexBindingDivisors = vertexBindingDivisorDescriptions.data();

	VkPipelineVertexInputStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pNext = &vertexInputDivisiorState;
	info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
	info.pVertexBindingDescriptions = vertexBindingDescriptions.data();
	info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
	info.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

	return info;
}

VkPipelineInputAssemblyStateCreateInfo VulkanVertexArray::CreateInputAssemblyState(PrimitiveTopology primitiveTopology) const
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};

	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = GetPipelinePrimitiveTopology(primitiveTopology);
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	return inputAssemblyState;
}

VkPipelineViewportStateCreateInfo VulkanVertexArray::CreateViewportState() const
{
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	return viewportState;
}

VkPipelineRasterizationStateCreateInfo VulkanVertexArray::CreateRasterizationState() const
{
	VkPipelineRasterizationStateCreateInfo rasterizationState{};

	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = RenderCommand::GetWireframeMode() ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = RenderCommand::GetFaceCulling() ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0.0f;
	rasterizationState.depthBiasClamp = 0.0f;
	rasterizationState.depthBiasSlopeFactor = 0.0f;
	rasterizationState.lineWidth = 1.0f;

	return rasterizationState;
}

VkPipelineMultisampleStateCreateInfo VulkanVertexArray::CreateMultisampleState() const
{
	VkPipelineMultisampleStateCreateInfo multisampleState{};

	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	return multisampleState;
}

VkPipelineDepthStencilStateCreateInfo VulkanVertexArray::CreateDepthStencilState() const
{
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};

	// TODO skip depth testing for now

	return depthStencilState;
}

VkPipelineColorBlendAttachmentState VulkanVertexArray::CreateColorBlendAttachmentState() const
{
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};

	colorBlendAttachmentState.blendEnable = VK_TRUE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	return colorBlendAttachmentState;
}

VkPipelineColorBlendStateCreateInfo VulkanVertexArray::CreateColorBlendState() const
{
	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachmentState;

	return colorBlendState;
}

VkPipelineDynamicStateCreateInfo VulkanVertexArray::CreatDynamicState() const
{
	VkPipelineDynamicStateCreateInfo dynamicState{};

	// TODO skip dynaimc state for now

	return dynamicState;
}

VkPipelineLayout VulkanVertexArray::CreatePipelineLayout() const
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	if (vkCreatePipelineLayout(vulkanContext->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		assert(false);
	}

	return pipelineLayout;
}

} // namespace Hedge
