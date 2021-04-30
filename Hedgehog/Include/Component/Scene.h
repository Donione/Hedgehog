#pragma once

#include <Component/Entity.h>
#include <entt.hpp>

#include <Renderer/Camera.h>


namespace Hedge
{

class Scene
{
public:
	Scene();

	Entity CreateEntity(const std::string& name);
	void DestroyEntity(Entity entity);

	void OnUpdate();

	void UpdateRenderSettings();

	const Entity GetPrimaryCamera();

private:
	Transform GetParentTransform(entt::entity child);


public:
	int plUsed = 3;
	
	// TODO this should be private
	// temporarily public so OnImGuiUpdate function can iterate over entities
	// TODO provide API for such things
	entt::registry registry;

private:
};

} // namespace Hedge
