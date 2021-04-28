#include <Renderer/Renderer.h>

#include <algorithm>


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

	auto& groups = vertexArray->GetGroups();

	// TODO this should be done only if camera position or transform changes between frames
	for (auto& [group, distance] : groups)
	{
		distance = glm::distance(sceneCamera.Get<Transform>().GetTranslation(),
									glm::vec3(transform * glm::vec4(group.center, 1.0f)));
	}

	auto comp = [](const std::pair<VertexGroup, float>& a, const std::pair<VertexGroup, float>& b)
	{
		return a.second > b.second;
	};
	std::sort(groups.begin(), groups.end(), comp);
	
	vertexArray->Bind();
	if (groups.empty())
	{
		RenderCommand::DrawIndexed(vertexArray);
	}
	else
	{
		for (const auto& [group, distance] : groups)
		{
			RenderCommand::DrawIndexed(vertexArray, group.endIndex - group.startIndex + 1, group.startIndex);
		}
	}
	vertexArray->Unbind();

	// Keep track of different shaders that are being used so they can be cleared at the end of the scene
	usedShaders.insert(vertexArray->GetShader());
}

} // namespace Hedge
