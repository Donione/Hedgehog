#include <Renderer/Renderer.h>


// static private member needs definition if not using C++17's inline static
//Camera Renderer::sceneCamera;

void Renderer::SetWireframeMode(bool enable)
{
	RenderCommand::SetWireframeMode(enable);
}

void Renderer::SetDepthTest(bool enable)
{
	RenderCommand::SetDepthTest(enable);
}

void Renderer::SetFaceCulling(bool enable)
{
	RenderCommand::SetFaceCulling(enable);
}

void Renderer::SetBlending(bool enable)
{
	RenderCommand::SetBlending(enable);
}

void Renderer::BeginScene(const Camera& camera)
{
	sceneCamera = camera;
}

void Renderer::EndScene()
{
}

void Renderer::Submit(const std::shared_ptr<Shader>& shader,
					  const std::shared_ptr<VertexArray>& vertexArray,
					  const glm::mat4x4& transform)
{
	shader->Bind();
	shader->UploadUniform("u_ViewProjection", sceneCamera.GetProjectionView());
	shader->UploadUniform("u_Transform", transform);

	RenderCommand::DrawIndexed(vertexArray);

	shader->Unbind();
}
