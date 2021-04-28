#pragma once

#include <Renderer/RendererAPI.h>
#include <Renderer/RenderContext.h>


namespace Hedge
{

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

	static bool GetWireframeMode() { return rendererAPI->GetWireframeMode(); }
	static bool GetDepthTest() { return rendererAPI->GetDepthTest(); }
	static bool GetFaceCulling() { return rendererAPI->GetFaceCulling(); }
	static bool GetBlending() { return rendererAPI->GetBlending(); }


	static void Resize(int width, int height, bool fillViewport = true)
	{
		rendererAPI->Resize(width, height, fillViewport);
	}

	static void SetViewport(int x, int y, int width, int height)
	{
		rendererAPI->SetViewport(x, y, width, height);
	}

	static void SetScissor(int x, int y, int width, int height)
	{
		rendererAPI->SetScissor(x, y, width, height);
	}


	static void SetClearColor(const glm::vec4& color)
	{
		rendererAPI->SetClearColor(color);
	}

	static void Begin()
	{
		rendererAPI->Begin();
	}

	static void End()
	{
		rendererAPI->End();
	}

	static void BeginFrame()
	{
		rendererAPI->BeginFrame();
	}

	static void EndFrame()
	{
		rendererAPI->EndFrame();
	}


	static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray,
							unsigned int count = 0,
							unsigned int offset = 0)
	{
		rendererAPI->DrawIndexed(vertexArray, count, offset);
	}

private:
	inline static RendererAPI* rendererAPI;
};

} // namespace Hedge
