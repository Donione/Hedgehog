#pragma once

#include <Component/Transform.h>


namespace Hedge
{

enum class LightType
{
	Point,
};

class Light
{
public:
	Light(LightType type = LightType::Point) : type(type) {}

	glm::vec3& GetColor() { return color; }
	const glm::vec3& GetPosition() const { return transform.GetTranslation(); }

	void SetColor(const glm::vec3& color) { this->color = color; }
	void SetPosition(const glm::vec3& position) { transform.SetTranslation(position); }

	Transform& GetTransform() { return transform; }

private:
	LightType type;
	glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
	Transform transform;
};

} // namespace Hedge
