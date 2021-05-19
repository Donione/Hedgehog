#include <Animation/Segment.h>


namespace Hedge
{

const Transform Segment::GetTransform(float timeStamp) const
{
	Transform transform;

	transform.SetTranslation(GetTranslation(timeStamp));
	transform.SetRotation(GetRotation(timeStamp));
	transform.SetScale(GetScale(timeStamp));

	return transform;
}

const glm::mat4 Segment::GetTransformMatrix(float timeStamp) const
{
	int firstKeyIndex = GetIndex<KeyPosition>(timeStamp, keyPositions);
	int secondKeyIndex = firstKeyIndex + 1;

	if (firstKeyIndex == -1)
	{
		return glm::mat4(1.0f);
	}

	KeyTransform firstKeyTransform = keyTransforms[firstKeyIndex];
	KeyTransform secondKeyTransform = keyTransforms[secondKeyIndex];
	float interpolant = GetInterpolant(firstKeyTransform.timeStamp, secondKeyTransform.timeStamp, timeStamp);

	glm::vec4 firstKeyPosition = firstKeyTransform.transform[3];
	glm::vec4 secondKeyPosition = secondKeyTransform.transform[3];
	glm::vec4 translation = glm::mix(firstKeyPosition, secondKeyPosition, interpolant);

	glm::quat firstKeyRotation = glm::quat_cast(firstKeyTransform.transform);
	glm::quat secondKeyRotation = glm::quat_cast(secondKeyTransform.transform);
	glm::quat rotation = glm::slerp(firstKeyRotation, secondKeyRotation, interpolant);

	glm::mat4 transform = glm::mat4_cast(rotation);
	transform[3] = translation;

	return transform;
}

const glm::vec3 Segment::GetTranslation(float timeStamp) const
{
	int firstKeyIndex = GetIndex<KeyPosition>(timeStamp, keyPositions);
	int secondKeyIndex = firstKeyIndex + 1;

	if (firstKeyIndex == -1)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	KeyPosition firstKeyPosition = keyPositions[firstKeyIndex];
	KeyPosition secondKeyPosition = keyPositions[secondKeyIndex];
	float interpolant = GetInterpolant(firstKeyPosition.timeStamp, secondKeyPosition.timeStamp, timeStamp);

	glm::vec3 translation = glm::mix(firstKeyPosition.position, secondKeyPosition.position, interpolant);

	return translation;
}

const glm::quat Segment::GetRotation(float timeStamp) const
{
	int firstKeyIndex = GetIndex<KeyRotation>(timeStamp, keyRotations);
	int secondKeyIndex = firstKeyIndex + 1;

	if (firstKeyIndex == -1)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	KeyRotation firstKeyRotation = keyRotations[firstKeyIndex];
	KeyRotation secondKeyRotation = keyRotations[secondKeyIndex];
	float interpolant = GetInterpolant(firstKeyRotation.timeStamp, secondKeyRotation.timeStamp, timeStamp);

	glm::quat rotation = glm::slerp(firstKeyRotation.rotation, secondKeyRotation.rotation, interpolant);
	rotation = glm::normalize(rotation);

	return rotation;
}

const glm::vec3 Segment::GetScale(float timeStamp) const
{
	int firstKeyIndex = GetIndex<KeyScale>(timeStamp, keyScales);
	int secondKeyIndex = firstKeyIndex + 1;

	if (firstKeyIndex == -1)
	{
		return glm::vec3(1.0f, 1.0f, 1.0f);
	}

	KeyScale firstKeyScale = keyScales[firstKeyIndex];
	KeyScale secondKeyScale = keyScales[secondKeyIndex];
	float interpolant = GetInterpolant(firstKeyScale.timeStamp, secondKeyScale.timeStamp, timeStamp);

	glm::vec3 scale = glm::mix(firstKeyScale.scale, secondKeyScale.scale, interpolant);

	return scale;
}

float Segment::GetInterpolant(float start, float end, float now) const
{
	float interpolant = 0.0f;

	float length = end - start;
	float elapsed = now - start;

	interpolant = elapsed / length;

	return interpolant;
}

} // namespace Hedge
