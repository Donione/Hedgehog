#pragma once

#include <Renderer/RendererAPI.h>

#include <Renderer/VulkanContext.h>


namespace Hedge
{

class VulkanRendererAPI : public RendererAPI
{
public:
	virtual void Init(RenderContext* renderContext) override;

	virtual void SetWireframeMode(bool enable) override { wireframeMode = enable; }
	virtual void SetDepthTest(bool enable) override { depthTest = enable; }
	virtual void SetFaceCulling(bool enable) override { faceCulling = enable; }
	virtual void SetBlending(bool enable) override { blending = enable; }

	virtual bool GetWireframeMode() const override { return wireframeMode; }
	virtual bool GetDepthTest() const override { return depthTest; }
	virtual bool GetFaceCulling() const override { return faceCulling; }
	virtual bool GetBlending() const override { return blending; }

	virtual void Resize(int width, int height, bool fillViewport = true) override;
	virtual void SetViewport(int x, int y, int width, int height) override;
	virtual void SetScissor(int x, int y, int width, int height) override;

	virtual void SetClearColor(const glm::vec4& color) override { clearColor = color; }
	virtual void Begin() override {}
	virtual void End() override;
	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray,
							 unsigned int count = 0,
							 unsigned int offset = 0) override;


	const VkViewport& GetViewport() const { return viewport; }
	const VkRect2D& GetScissor() const { return scissor; }

private:
	VulkanContext* renderContext;

	bool wireframeMode = false;
	bool depthTest = false;
	bool faceCulling = false;
	bool blending = false;

	glm::vec4 clearColor;
	VkViewport viewport = {};
	VkRect2D scissor = {};
};

} // namespace Hedge