#include <Component/Entity.h>


namespace Hedge
{

Entity Entity::CreateChild(const std::string& name)
{
	// Create child
	entt::entity childEntity = registry->create();
	registry->emplace<std::string>(childEntity, name);
	Entity child(childEntity, registry);

	// Since an entity can't have multiple components of the same type
	// we keep track only of the Child to Parent relationship as a Parent component for the Child
	child.Add<Parent>(*this);

	return child;
}

Entity Entity::AddChild(Entity& child)
{
	// An entity can have at most one parent
	assert(!child.Has<Parent>());

	child.Add<Parent>(*this);

	return child;
}

} // namespace Hedge
