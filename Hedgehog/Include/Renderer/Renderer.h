#pragma once

#include <Renderer/RendererAPI.h>
#include <Renderer/RenderCommand.h>

#include <Renderer/Buffer.h>
#include <Renderer/VertexArray.h>
#include <Renderer/Shader.h>
#include <Renderer/Camera.h>


class Renderer
{
public:
	static void BeginScene(const Camera& camera);
	static void EndScene();

	static void Submit(const std::shared_ptr<VertexArray>& vertexArray);

	static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

private:
	// C++17 has inline static for static member definition
	inline static Camera sceneCamera;
};
