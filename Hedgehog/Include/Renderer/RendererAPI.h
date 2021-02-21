#pragma once

#include <memory>
#include <glm/glm.hpp>

#include <Renderer/RenderContext.h>
#include <Renderer/VertexArray.h>


class RendererAPI
{
public:
	enum class API
	{
		None = 0,
		OpenGL = 1,
		DirectX12 = 2,
	};

public:
	virtual void Init(RenderContext* renderContext) = 0;

	virtual void SetWireframeMode(bool enable) = 0;
	virtual void SetDepthTest(bool enable) = 0;
	virtual void SetFaceCulling(bool enable) = 0;
	virtual void SetBlending(bool enable) = 0;

	virtual bool GetWireframeMode() const = 0;
	virtual bool GetDepthTest() const = 0;
	virtual bool GetFaceCulling() const = 0;
	virtual bool GetBlending() const = 0;

	virtual void SetViewport(int width, int height) = 0;

	virtual void SetClearColor(const glm::vec4& color) = 0;
	virtual void Begin() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;

	static API GetAPI() { return api; }

private:
	static API api;
};
