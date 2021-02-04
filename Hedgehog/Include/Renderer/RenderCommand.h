#pragma once

#include <Renderer/RendererAPI.h>
#include <Renderer/RenderContext.h>


class RenderCommand
{
public:
	// TODECIDE maybe insted of tying render context and rendererAPI together
	// create a static render context instance getter
	static void Init(RenderContext* renderContext);

	static void SetWireframeMode(bool enable)
	{
		rendererAPI->SetWireframeMode(enable);
	}

	static void SetDepthTest(bool enable)
	{
		rendererAPI->SetDepthTest(enable);
	}

	static void SetFaceCulling(bool enable)
	{
		rendererAPI->SetFaceCulling(enable);
	}

	static void SetBlending(bool enable)
	{
		rendererAPI->SetBlending(enable);
	}


	static void SetViewport(int width, int height)
	{
		rendererAPI->SetViewport(width, height);
	}


	static void SetClearColor(const glm::vec4& color)
	{
		rendererAPI->SetClearColor(color);
	}

	static void Clear()
	{
		rendererAPI->Clear();
	}


	static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		rendererAPI->DrawIndexed(vertexArray);
	}

private:
	static RendererAPI* rendererAPI;
};
