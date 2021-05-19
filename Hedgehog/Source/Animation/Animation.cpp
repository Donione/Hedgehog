#include <Animation/Animation.h>

#include <glm/gtc/matrix_transform.hpp>


namespace Hedge
{

Animation::Animation()
{
	duration = 10.0f;
	rootSegmentIndex = 0;

	segments.push_back({ Segment("palm", 0) , -1 } );
	segments.push_back({ Segment("pinky1", 1), 0 } );
	segments.push_back({ Segment("pinky2", 2), 1 } );

	transforms.resize(segments.size());

	segments[0].first.offset = glm::mat4(1.0f);
	segments[0].first.keyPositions.push_back({  0.0f, { 0.0f, 1.5f, 0.0f } });
	segments[0].first.keyPositions.push_back({  5.0f, { 0.0f, 1.5f, 0.0f } });
	segments[0].first.keyPositions.push_back({ 10.0f, { 0.0f, 1.5f, 0.0f } });
	segments[0].first.keyRotations.push_back({  0.0f, glm::quat(glm::radians(glm::vec3( 0.0f, 0.0f, 0.0f))) });
	segments[0].first.keyRotations.push_back({  5.0f, glm::quat(glm::radians(glm::vec3(30.0f, 0.0f, 0.0f))) });
	segments[0].first.keyRotations.push_back({ 10.0f, glm::quat(glm::radians(glm::vec3( 0.0f, 0.0f, 0.0f))) });
	segments[0].first.keyScales.push_back({  0.0f, { 1.0f, 1.0f, 1.0f } });
	segments[0].first.keyScales.push_back({  5.0f, { 1.0f, 1.0f, 1.0f } });
	segments[0].first.keyScales.push_back({ 10.0f, { 1.0f, 1.0f, 1.0f } });

	segments[1].first.offset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f));
	segments[1].first.keyPositions.push_back({  0.0f, { 0.0f, 1.5f, 0.0f } });
	segments[1].first.keyPositions.push_back({  5.0f, { 0.0f, 1.5f, 0.0f } });
	segments[1].first.keyPositions.push_back({ 10.0f, { 0.0f, 1.5f, 0.0f } });
	segments[1].first.keyRotations.push_back({  0.0f, glm::quat(glm::radians(glm::vec3( 0.0f,  0.0f, 0.0f))) });
	segments[1].first.keyRotations.push_back({  5.0f, glm::quat(glm::radians(glm::vec3(30.0f, 30.0f, 0.0f))) });
	segments[1].first.keyRotations.push_back({ 10.0f, glm::quat(glm::radians(glm::vec3( 0.0f,  0.0f, 0.0f))) });
	segments[1].first.keyScales.push_back({  0.0f, { 1.0f, 1.0f, 1.0f } });
	segments[1].first.keyScales.push_back({  5.0f, { 1.0f, 1.0f, 1.0f } });
	segments[1].first.keyScales.push_back({ 10.0f, { 1.0f, 1.0f, 1.0f } });

	segments[2].first.offset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f));
	segments[2].first.keyPositions.push_back({  0.0f, { 0.0f, 1.5f, 0.0f } });
	segments[2].first.keyPositions.push_back({  5.0f, { 0.0f, 1.5f, 0.0f } });
	segments[2].first.keyPositions.push_back({ 10.0f, { 0.0f, 1.5f, 0.0f } });
	segments[2].first.keyRotations.push_back({  0.0f, glm::quat(glm::radians(glm::vec3( 0.0f,  0.0f, 0.0f))) });
	segments[2].first.keyRotations.push_back({  5.0f, glm::quat(glm::radians(glm::vec3(30.0f, 30.0f, 0.0f))) });
	segments[2].first.keyRotations.push_back({ 10.0f, glm::quat(glm::radians(glm::vec3( 0.0f,  0.0f, 0.0f))) });
	segments[2].first.keyScales.push_back({  0.0f, { 1.0f, 1.0f, 1.0f } });
	segments[2].first.keyScales.push_back({  5.0f, { 1.0f, 1.0f, 1.0f } });
	segments[2].first.keyScales.push_back({ 10.0f, { 1.0f, 1.0f, 1.0f } });
}

Animation::Animation(const std::vector<std::pair<Segment, int>>& segments)
{
	// TODO ideally segments should be sorted in a breadth first fashion
	this->segments = segments;
	duration = segments.back().first.keyPositions.back().timeStamp;
	transforms.resize(segments.size());

	// Find the root segment
	// Assume there is exactly one root segment
	for (auto segment = segments.begin(); segment != segments.end(); ++segment)
	{
		if (segment->second == -1)
		{
			rootSegmentIndex = static_cast<int>(std::distance(segments.begin(), segment));
			break;
		}
	}
	assert(rootSegmentIndex != -1);
}

const std::vector<glm::mat4>& Animation::GetTransforms(float timeStamp)
{
	CalculateTransforms(timeStamp, rootSegmentIndex);

	return transforms;
}

void Animation::CalculateTransforms(float timeStamp,
									int segmentIndex,
									const Transform& parentTransform)
{
	Segment& segment = segments[segmentIndex].first;

	Transform finalTransform = parentTransform * segment.GetTransform(timeStamp);
	transforms[segment.GetID()] = finalTransform.Get() * segment.offset;

	for (auto segment = segments.begin(); segment != segments.end(); ++segment)
	{
		if (segment->second == segmentIndex)
		{
			CalculateTransforms(timeStamp,
								static_cast<int>(std::distance(segments.begin(), segment)),
								finalTransform);
		}
	}
}

void Animation::CalculateTransformMatrices(float timeStamp,
										   int segmentIndex,
										   const glm::mat4& parentTransform)
{
	Segment& segment = segments[segmentIndex].first;

	glm::mat4 finalTransform = parentTransform * segment.GetTransformMatrix(timeStamp);
	transforms[segment.GetID()] = finalTransform * segment.offset;

	for (auto segment = segments.begin(); segment != segments.end(); ++segment)
	{
		if (segment->second == segmentIndex)
		{
			CalculateTransformMatrices(timeStamp,
									   static_cast<int>(std::distance(segments.begin(), segment)),
									   finalTransform);
		}
	}
}

} // namespace Hedge
