#pragma once

#include <Animation/Segment.h>


namespace Hedge
{

class Animation
{
public:
	Animation();
	Animation(const std::vector<std::pair<Segment, int>>& segments);

	float GetDuration() const { return duration; }
	const std::vector<glm::mat4>& GetTransforms(float timeStamp);

private:
	void CalculateTransforms(float timeStamp,
							 int segmentIndex = 0,
							 const Transform& parentTransform = Transform());

	void CalculateTransformMatrices(float timeStamp,
									int segmentIndex = 0,
									const glm::mat4& parentTransform = glm::mat4(1.0f));


private:
	float duration;
	int rootSegmentIndex = -1;
	// pairs of segement and its parent
	std::vector<std::pair<Segment, int>> segments;
	std::vector<glm::mat4> transforms;
};

} // namespace Hedge
