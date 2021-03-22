#include <Component/Scene.h>

#include <Component/Mesh.h>
#include <Component/Transform.h>

#include <Renderer/Renderer.h>
#include <Renderer/DirectX12VertexArray.h>


namespace Hedge
{

Scene::Scene()
{
	auto group = registry.group<Hedge::Mesh, Hedge::Transform>();
}

Entity Scene::CreateEntity(const std::string& name)
{
	entt::entity entity =  registry.create();
	registry.emplace<std::string>(entity, name);
	return Entity(entity, &registry);
}

void Scene::DestroyEntity(Entity entity)
{
	registry.destroy(entity.entity);
}

void Scene::OnUpdate()
{
	auto group = registry.group<Hedge::Mesh, Hedge::Transform>();

	auto directionalLights = registry.view<Hedge::DirectionalLight>();
	auto pointLights = registry.view<Hedge::PointLight>();
	auto spotLights = registry.view<Hedge::SpotLight>();

	for (auto [entity, mesh, transform] : group.each())
	{
		std::string name = registry.get<std::string>(entity);

		if (mesh.enabled)
		{
			mesh.GetShader()->UploadConstant("u_viewPos", cameraTransform->GetTranslation());
			mesh.GetShader()->UploadConstant("u_directionalLight", directionalLights.raw(), (int)directionalLights.size());
			mesh.GetShader()->UploadConstant("u_numberOfPointLights", plUsed);
			mesh.GetShader()->UploadConstant("u_pointLight", pointLights.raw(), (int)pointLights.size());
			mesh.GetShader()->UploadConstant("u_spotLight", spotLights.raw(), (int)spotLights.size());

			if (registry.has<Hedge::PointLight>(entity))
			{
				mesh.GetShader()->UploadConstant("u_lightColor", registry.get<Hedge::PointLight>(entity).color);
			}
			else if (registry.has<Hedge::SpotLight>(entity))
			{
				mesh.GetShader()->UploadConstant("u_lightColor", registry.get<Hedge::SpotLight>(entity).color);
			}

			Hedge::Renderer::Submit(mesh.Get(), transform.Get());
		}
	}
}

void Scene::UpdateRenderSettings()
{
	auto view = registry.view<Hedge::Mesh>();
	for (auto [entity, mesh] : view.each())
	{
		std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(mesh.Get())->UpdateRenderSettings();
	}
}

} // namespace Hedge
