#pragma once

#include <Renderer/RendererAPI.h>
#include <Renderer/DirectX12Context.h>

#include <d3dx12.h>


class DirectX12RendererAPI : public RendererAPI
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

	virtual void SetViewport(int width, int height) override;

	virtual void SetClearColor(const glm::vec4& color) override { clearColor = color; }
	virtual void Begin() override;
	virtual void End() override;
	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;

private:
	DirectX12Context* renderContext;

	bool wireframeMode = false;
	bool depthTest = false;
	bool faceCulling = false;
	bool blending = false;

	glm::vec4 clearColor;
	CD3DX12_VIEWPORT viewport = {};
	CD3DX12_RECT scissorRect = {};

	DirectX12Context::FrameContext* frameCtxt;
	UINT backBufferIdx;
	D3D12_RESOURCE_BARRIER barrier;
};
