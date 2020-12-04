#include <Renderer/Renderer.h>


void Renderer::BeginScene()
{
}

void Renderer::EndScene()
{
}

void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray)
{
	RenderCommand::DrawIndexed(vertexArray);
}
