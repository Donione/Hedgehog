#pragma once

#include <Animation/Segment.h>


namespace Hedge
{

class Animation
{
public:
	Animation(const std::string& filename);

	const std::vector<glm::mat4>& GetTransforms(float timeStamp);

private:
	void CalculateTransforms(float timeStamp,
							 int segmentIndex = 0,
							 const Transform& parentTransform = Transform());


private:
	// pairs of segement and its parent
	std::vector<std::pair<Segment, int>> segments;
	std::vector<glm::mat4> transforms;
};

} // namespace Hedge
