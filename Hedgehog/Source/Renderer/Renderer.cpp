#include <Renderer/Renderer.h>


namespace Hedge
{

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

void Renderer::BeginScene(Entity camera)
{
	sceneCamera = camera;
}

void Renderer::EndScene()
{
	for (auto& shader : usedShaders)
	{
		// When we unbind the shader, we clear the number of objects that used the same shader
		// This is used in DirectX12 implemenation to offset the constant buffers
		// OpenGL doesn't care
		shader->Unbind();
	}

	usedShaders.clear();
}

void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray,
					  const glm::mat4x4& transform)
{
	if (!sceneCamera)
	{
		return;
	}

	if (!usedShaders.contains(vertexArray->GetShader()))
	{
		auto projectionView = sceneCamera.Get<Camera>().GetProjection() * glm::inverse(sceneCamera.Get<Transform>().Get());
		vertexArray->GetShader()->UploadConstant("u_ViewProjection", projectionView);
	}
	vertexArray->GetShader()->UploadConstant("u_Transform", transform);

	RenderCommand::DrawIndexed(vertexArray);

	// Keep track of different shaders that are being used so they can be cleared at the end of the scene
	usedShaders.insert(vertexArray->GetShader());
}

} // namespace Hedge
