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

public:
	int plUsed = 3;

	Camera* camera = nullptr;
	Transform* cameraTransform = nullptr;


public:
	entt::registry registry;
};

} // namespace Hedge
