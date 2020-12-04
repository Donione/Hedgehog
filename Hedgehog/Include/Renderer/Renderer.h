#pragma once

#include <Renderer/RendererAPI.h>
#include <Renderer/RenderCommand.h>

#include <Renderer/Buffer.h>
#include <Renderer/VertexArray.h>
#include <Renderer/Shader.h>


class Renderer
{
public:
	static void BeginScene();
	static void EndScene();

	static void Submit(const std::shared_ptr<VertexArray>& vertexArray);

	static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
};
