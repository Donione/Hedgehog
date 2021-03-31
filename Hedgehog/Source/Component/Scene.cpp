#include <Component/Scene.h>

#include <Component/Mesh.h>
#include <Component/Transform.h>

#include <Renderer/Renderer.h>
#include <Renderer/DirectX12VertexArray.h>


namespace Hedge
{

Scene::Scene()
{
	// As per recommendation in https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#groups
	// prepare groups that will be used while the registry is still empty
	auto group = registry.group<Mesh, Transform>();
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
	auto& cameraTransform = registry.get<Transform>(registry.view<Camera>().front());

	auto group = registry.group<Mesh, Transform>();

	auto directionalLights = registry.view<DirectionalLight>();
	auto pointLights = registry.view<PointLight>();
	auto spotLights = registry.view<SpotLight>();

	for (auto [entity, mesh, transform] : group.each())
	{
		std::string name = registry.get<std::string>(entity);

		if (mesh.enabled)
		{
			mesh.GetShader()->UploadConstant("u_viewPos", cameraTransform.GetTranslation());
			mesh.GetShader()->UploadConstant("u_directionalLight", directionalLights.raw(), (int)directionalLights.size());
			mesh.GetShader()->UploadConstant("u_numberOfPointLights", plUsed);
			mesh.GetShader()->UploadConstant("u_pointLight", pointLights.raw(), (int)pointLights.size());
			mesh.GetShader()->UploadConstant("u_spotLight", spotLights.raw(), (int)spotLights.size());

			if (registry.has<PointLight>(entity))
			{
				mesh.GetShader()->UploadConstant("u_lightColor", registry.get<PointLight>(entity).color);
			}
			else if (registry.has<SpotLight>(entity))
			{
				mesh.GetShader()->UploadConstant("u_lightColor", registry.get<SpotLight>(entity).color);
			}

			Renderer::Submit(mesh.Get(), transform.Get());
		}
	}
}

void Scene::UpdateRenderSettings()
{
	auto view = registry.view<Mesh>();
	for (auto [entity, mesh] : view.each())
	{
		std::dynamic_pointer_cast<DirectX12VertexArray>(mesh.Get())->UpdateRenderSettings();
	}
}

const Entity Scene::GetPrimaryCamera()
{
	auto view = registry.view<Camera>();
	for (auto [entity, camera] : view.each())
	{
		// For now, let's ignore if there are multiple primary cameras, just return the first one we find
		if (camera.IsPrimary())
		{
			return { entity, &registry };
		}
	}

	return { entt::null, nullptr };
}

} // namespace Hedge
