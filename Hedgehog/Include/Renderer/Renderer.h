#pragma once

#include <Renderer/RendererAPI.h>
#include <Renderer/RenderCommand.h>

#include <Renderer/Buffer.h>
#include <Renderer/VertexArray.h>
#include <Renderer/Shader.h>
#include <Renderer/Camera.h>
#include <Renderer/Texture.h>

#include <set>


class Renderer
{
public:
	static void SetWireframeMode(bool enable);
	static void SetDepthTest(bool enable);
	static void SetFaceCulling(bool enable);
	static void SetBlending(bool enable);

	static void BeginScene(const Camera& camera);
	static void EndScene();

	static void Submit(const std::shared_ptr<VertexArray>& vertexArray,
					   const glm::mat4x4& transform = glm::mat4x4(1.0f));

	static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

private:
	// C++17 has inline static for static member definition
	inline static Camera sceneCamera;

	inline static std::set<std::shared_ptr<Shader>> usedShaders;
};
