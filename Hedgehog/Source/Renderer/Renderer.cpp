#include <Renderer/Renderer.h>


// static private member needs definition if not using C++17's inline static
//Camera Renderer::sceneCamera;

void Renderer::BeginScene(const Camera& camera)
{
	sceneCamera = camera;
}

void Renderer::EndScene()
{
}

void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray)
{
	RenderCommand::DrawIndexed(vertexArray);
}
