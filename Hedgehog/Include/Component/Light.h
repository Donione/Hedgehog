#pragma once

#include <Component/Transform.h>


namespace Hedge
{

// Light structures padded acording to DirectX constant buffer packing rules (buckets of float4s)

// PointLight shines in all directions and gets attenuated with distance
struct PointLight
{
	glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
	float padding0;
	glm::vec3 position = glm::vec3(0.0f);
	float padding1;
	glm::vec3 attenuation = glm::vec3(1.0f, 0.0f, 0.0f); // x = constant, y = linear, z = quadratic components
	float padding2;
};

// SpotLight is a PointLight with a direction and cutoff radius (and a soft edge)
struct SpotLight
{
	glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
	float padding0;
	glm::vec3 position = glm::vec3(0.0f);
	float padding1;
	glm::vec3 attenuation = glm::vec3(1.0f, 0.0f, 0.0f); // x = constant, y = linear, z = quadratic components
	float padding2;
	glm::vec3 direction = glm::vec3(0.0f);
	float padding3;
	glm::vec2 cutoffAngle = glm::cos(glm::radians(glm::vec2(15.0f, 20.0f))); // cosine of inner and outer angle in radians
	float padding4[2];
};

// Directional Light is a sun-like light where position doesn't matter
struct DirectionalLight
{
	glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
	float padding0;
	glm::vec3 direction = glm::vec3(0.0f);
	float padding1;
};

} // namespace Hedge
