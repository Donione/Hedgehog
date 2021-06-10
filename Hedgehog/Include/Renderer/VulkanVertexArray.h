#pragma once

#include <Renderer/VertexArray.h>

#include <Renderer/VulkanShader.h>
#include <Renderer/VulkanContext.h>


namespace Hedge
{

class VulkanVertexArray : public VertexArray
{
public:
	VulkanVertexArray(const std::shared_ptr<Shader>& inputShader,
					  PrimitiveTopology primitiveTopology,
					  const BufferLayout& inputLayout,
					  const std::vector<Hedge::TextureDescription>& textureDescriptions);
	virtual ~VulkanVertexArray() override;

	virtual void Bind() override;
	virtual void Unbind() const override { /* do nothing */ }

	virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
	virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
	virtual void AddTexture(TextureType type, int position, const std::shared_ptr<Texture>& texture) override;
	virtual void SetupGroups(const std::vector<VertexGroup>& groups) override;
	virtual void SetInstanceCount(unsigned int instanceCount) override { this->instanceCount = instanceCount; }

	virtual const std::shared_ptr<Shader> GetShader() const override { return baseShader; }
	virtual PrimitiveTopology GetPrimitiveTopology() const override { return primitiveTopology; }
	virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return vertexBuffers; }
	virtual const std::shared_ptr<IndexBuffer> GetIndexBuffer() const override { return indexBuffer; }
	virtual std::vector<std::pair<VertexGroup, float>>& GetGroups() override { return groups; }
	virtual unsigned int GetInstanceCount() const override { return instanceCount; }


	void Resize(int width, int height);

private:
	void CreatePipeline();
	VkPipelineVertexInputStateCreateInfo CreateVertexInputState() const;
	VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyState(PrimitiveTopology primitiveTopology) const;
	VkPipelineViewportStateCreateInfo CreateViewportState() const;
	VkPipelineRasterizationStateCreateInfo CreateRasterizationState() const;
	VkPipelineMultisampleStateCreateInfo CreateMultisampleState() const;
	VkPipelineDepthStencilStateCreateInfo CreateDepthStencilState() const;
	VkPipelineColorBlendAttachmentState CreateColorBlendAttachmentState() const;
	VkPipelineColorBlendStateCreateInfo CreateColorBlendState() const;
	VkPipelineDynamicStateCreateInfo CreatDynamicState() const;
	VkPipelineLayout CreatePipelineLayout() const;

	VkPrimitiveTopology GetPipelinePrimitiveTopology(PrimitiveTopology topology) const; // { return pipelinePrimitiveTopologies[(int)topology]; }



private:
	PrimitiveTopology primitiveTopology;
	BufferLayout bufferLayout;
	std::vector<Hedge::TextureDescription> textureDescriptions;
	
	unsigned int instanceCount = 1;

	std::shared_ptr<Shader> baseShader;
	std::shared_ptr<VulkanShader> shader;
	std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
	std::shared_ptr<IndexBuffer> indexBuffer;
	std::vector<std::pair<VertexGroup, float>> groups;
	std::vector<std::shared_ptr<Texture>> textures;

	VkPipeline pipeline = VK_NULL_HANDLE;

	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	//VkPipelineTessellationStateCreateInfo tessellationState;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizationState;
	VkPipelineMultisampleStateCreateInfo multisampleState;
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
	VkPipelineColorBlendStateCreateInfo colorBlendState;
	VkPipelineDynamicStateCreateInfo dynamicState;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	VulkanContext* vulkanContext = nullptr;

	VkViewport viewport{};
	VkRect2D scissor{};

	// TODO WTF why is this array not initialized to the values written here?
	const VkPrimitiveTopology pipelinePrimitiveTopologies[4]
	{
		VK_PRIMITIVE_TOPOLOGY_MAX_ENUM, // None / Undefined
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};
};

} // namespace Hedge
