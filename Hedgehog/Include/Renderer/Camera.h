#pragma once

#include <Component/Transform.h>


namespace Hedge
{

struct Frustum
{
	float left;
	float right;
	float bottom;
	float top;
	// Fun Fact: near and far are defined as nothing through windows.h (minwindef.h)
	float nearClip;
	float farClip;

	// fov and aspect ratio can be computed from left, right, top and bottom and vice versa
	float fov;
	float aspectRatio;

	Frustum() = default;
};

class Camera
{
public:
	Camera() = default;

public:
	const glm::vec3& GetPosition() const { return transform.GetTranslation(); }
	void SetPosition(const glm::vec3& position);

	const glm::vec3& GetRotation() const { return transform.GetRotation(); }
	void SetRotation(const glm::vec3& rotation);

	void Move(const glm::vec3& positionOffset);
	void Rotate(const glm::vec3& rotationOffset);

	void SetAspectRatio(float aspectRatio);

	const glm::mat4x4& GetView() const { return view; }
	const glm::mat4x4& GetProjection() const { return projection; }
	const glm::mat4x4& GetProjectionView() const { return projectionView; }

protected:
	void CalculateView();


protected:
	Transform transform;

	glm::mat4x4 view = glm::mat4x4(1.0f); // inverse of model (transform) matrix
	glm::mat4x4 projection = glm::mat4x4(1.0f);
	glm::mat4x4 projectionView = glm::mat4x4(1.0f);

	Frustum frustum = Frustum();
};


class OrthographicCamera : public Camera
{
public:
	OrthographicCamera() = default;
	OrthographicCamera(float left, float right, float bottom, float top);
	OrthographicCamera(float left, float right, float bottom, float top, float frustumNear, float frustumFar);
};


class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera() = default;
	PerspectiveCamera(float fov, float aspectRatio, float frustumNear, float frustumFar);
};

} // namespace Hedge
