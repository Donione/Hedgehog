#include <Animation/Animation.h>

#include <glm/gtc/matrix_transform.hpp>


namespace Hedge
{

Animation::Animation(const std::string& filename)
{
	segments.push_back({ Segment("palm", 0) , -1 } );
	segments.push_back({ Segment("pinky1", 1), 0 } );
	segments.push_back({ Segment("pinky2", 2), 1 } );

	transforms.resize(segments.size());

	segments[0].first.offset = glm::mat4(1.0f);
	segments[0].first.keyPositions.push_back({  0.0f, { 0.0f, 1.5f, 0.0f } });
	segments[0].first.keyPositions.push_back({  5.0f, { 0.0f, 1.5f, 0.0f } });
	segments[0].first.keyPositions.push_back({ 10.0f, { 0.0f, 1.5f, 0.0f } });
	segments[0].first.keyRotations.push_back({  0.0f, {  0.0f, 0.0f, 0.0f } });
	segments[0].first.keyRotations.push_back({  5.0f, { 30.0f, 0.0f, 0.0f } });
	segments[0].first.keyRotations.push_back({ 10.0f, {  0.0f, 0.0f, 0.0f } });
	segments[0].first.keyScales.push_back({  0.0f, { 1.0f, 1.0f, 1.0f } });
	segments[0].first.keyScales.push_back({  5.0f, { 1.0f, 1.0f, 1.0f } });
	segments[0].first.keyScales.push_back({ 10.0f, { 1.0f, 1.0f, 1.0f } });

	segments[1].first.offset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f));
	segments[1].first.keyPositions.push_back({  0.0f, { 0.0f, 1.5f, 0.0f } });
	segments[1].first.keyPositions.push_back({  5.0f, { 0.0f, 1.5f, 0.0f } });
	segments[1].first.keyPositions.push_back({ 10.0f, { 0.0f, 1.5f, 0.0f } });
	segments[1].first.keyRotations.push_back({  0.0f, {  0.0f, 0.0f, 0.0f } });
	segments[1].first.keyRotations.push_back({  5.0f, { 30.0f, 30.0f, 0.0f } });
	segments[1].first.keyRotations.push_back({ 10.0f, {  0.0f, 0.0f, 0.0f } });
	segments[1].first.keyScales.push_back({  0.0f, { 1.0f, 1.0f, 1.0f } });
	segments[1].first.keyScales.push_back({  5.0f, { 1.0f, 1.0f, 1.0f } });
	segments[1].first.keyScales.push_back({ 10.0f, { 1.0f, 1.0f, 1.0f } });

	segments[2].first.offset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f));
	segments[2].first.keyPositions.push_back({  0.0f, { 0.0f, 1.5f, 0.0f } });
	segments[2].first.keyPositions.push_back({  5.0f, { 0.0f, 1.5f, 0.0f } });
	segments[2].first.keyPositions.push_back({ 10.0f, { 0.0f, 1.5f, 0.0f } });
	segments[2].first.keyRotations.push_back({  0.0f, {  0.0f, 0.0f, 0.0f } });
	segments[2].first.keyRotations.push_back({  5.0f, { 30.0f, 30.0f, 0.0f } });
	segments[2].first.keyRotations.push_back({ 10.0f, {  0.0f, 0.0f, 0.0f } });
	segments[2].first.keyScales.push_back({  0.0f, { 1.0f, 1.0f, 1.0f } });
	segments[2].first.keyScales.push_back({  5.0f, { 1.0f, 1.0f, 1.0f } });
	segments[2].first.keyScales.push_back({ 10.0f, { 1.0f, 1.0f, 1.0f } });
}

const std::vector<glm::mat4>& Animation::GetTransforms(float timeStamp)
{
	CalculateTransforms(timeStamp);
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

} // namespace Hedge
