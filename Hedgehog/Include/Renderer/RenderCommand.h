#pragma once

#include <Renderer/RendererAPI.h>


class RenderCommand
{
public:
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
