#pragma once

#include <entt.hpp>


namespace Hedge
{

class Entity
{
public:
	Entity() = default;
	Entity(entt::entity entity, entt::registry* registry) : entity(entity), registry(registry) {}

	bool Is(const entt::entity& other) const { return entity == other; }

	template<typename Component, typename... Args>
	Component& Add(Args &&... args)
	{
		return registry->emplace<Component>(entity, std::forward<Args>(args)...);
	}

	template<typename Component, typename... Args>
	Component& Replace(Args &&... args)
	{
		assert(Has<Component>());
		return registry->replace<Component>(entity, std::forward<Args>(args)...);
	}

	template<typename Component>
	void Remove()
	{
		assert(Has<Component>());

		registry->remove<Component>(entity);
	}

	template<typename Component>
	Component& Get() const
	{
		assert(Has<Component>());

		return registry->get<Component>(entity);

	}
	
	template<typename Component>
	bool Has() const
	{
		return registry->has<Component>(entity);
	}

private:
	entt::entity entity = entt::null;
	entt::registry* registry = nullptr;

	friend class Scene;
};

} // namespace Hedge
