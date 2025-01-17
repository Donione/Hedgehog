#pragma once

#include <Component/Transform.h>

#include <string>
#include <vector>

#include <glm/gtc/quaternion.hpp>


namespace Hedge
{

struct KeyPosition
{
	float timeStamp;
	glm::vec3 position;
};

struct KeyRotation
{
	float timeStamp;
	glm::quat rotation;
};

struct KeyScale
{
	float timeStamp;
	glm::vec3 scale;
};

struct KeyTransform
{
	float timeStamp;
	glm::mat4 transform;
};


class Segment
{
public:
	Segment(const std::string& name, int ID) : name(name), ID(ID) {}

	int GetID() const { return ID; }
	const std::string& GetName() const { return name; }
	const Transform GetTransform(float timeStamp) const;
	const glm::mat4 GetTransformMatrix(float timeStamp) const;

private:
	const glm::vec3 GetTranslation(float timeStamp) const;
	const glm::quat GetRotation(float timeStamp) const;
	const glm::vec3 GetScale(float timeStamp) const;

	template <typename T>
	int GetIndex(float timeStamp, const std::vector<T>& elements) const
	{
		for (auto it = elements.begin(); it != elements.end(); ++it)
		{
			if (timeStamp < it->timeStamp)
			{
				return (int)std::distance(elements.begin(), it) - 1;
			}
		}
		return -1;
	}

	float GetInterpolant(float start, float end, float now) const;


public:
	glm::mat4 offset{1.0f};

	std::vector<KeyPosition> keyPositions;
	std::vector<KeyRotation> keyRotations;
	std::vector<KeyScale> keyScales;

	std::vector<KeyTransform> keyTransforms;

private:
	std::string name;
	int ID;
};

} // namespace Hedge
