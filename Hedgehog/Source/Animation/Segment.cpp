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

const glm::vec3 Segment::GetTranslation(float timeStamp) const
{
	int firstKeyIndex = GetIndex<KeyPosition>(timeStamp, keyPositions);
	int secondKeyIndex = firstKeyIndex + 1;

	KeyPosition firstKeyPosition = keyPositions[firstKeyIndex];
	KeyPosition secondKeyPosition = keyPositions[secondKeyIndex];
	float interpolant = GetInterpolant(firstKeyPosition.timeStamp, secondKeyPosition.timeStamp, timeStamp);

	glm::vec3 translation = glm::mix(firstKeyPosition.position, secondKeyPosition.position, interpolant);

	return translation;
}

const glm::vec3 Segment::GetRotation(float timeStamp) const
{
	int firstKeyIndex = GetIndex<KeyRotation>(timeStamp, keyRotations);
	int secondKeyIndex = firstKeyIndex + 1;

	KeyRotation firstKeyAngle = keyRotations[firstKeyIndex];
	KeyRotation secondKeyAngle = keyRotations[secondKeyIndex];
	float interpolant = GetInterpolant(firstKeyAngle.timeStamp, secondKeyAngle.timeStamp, timeStamp);

	glm::vec3 rotation = glm::mix(firstKeyAngle.angle, secondKeyAngle.angle, interpolant);

	return rotation;
}

const glm::vec3 Segment::GetScale(float timeStamp) const
{
	int firstKeyIndex = GetIndex<KeyScale>(timeStamp, keyScales);
	int secondKeyIndex = firstKeyIndex + 1;

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
