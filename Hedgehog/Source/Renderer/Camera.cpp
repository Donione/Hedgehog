#include <Renderer/Camera.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace Hedge
{

void Camera::SetPosition(const glm::vec3& position)
{
	this->position = position;

	CalculateView();
}

void Camera::SetRotation(const glm::vec3& rotation)
{
	this->rotationAngles = rotation;

	this->rotation = glm::rotate(glm::mat4x4(1.0f), glm::radians(rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::rotate(glm::mat4x4(1.0f), glm::radians(rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::mat4x4(1.0f), glm::radians(rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));

	CalculateView();
}

void Camera::Move(const glm::vec3& positionOffset)
{
	glm::vec4 orientedPositionOffset = rotation * glm::vec4(positionOffset, 1.0f);

	position += glm::vec3(orientedPositionOffset);

	CalculateView();
}

void Camera::Rotate(const glm::vec3& rotationOffset)
{
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.z), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::extractEulerAngleXYZ(rotation, rotationAngles.x, rotationAngles.y, rotationAngles.z);
	rotationAngles = glm::degrees(rotationAngles);

	CalculateView();
}

void Camera::SetAspectRatio(float aspectRatio)
{
	frustum.aspectRatio = aspectRatio;

	projection = glm::perspective(glm::radians(frustum.fov), aspectRatio, frustum.nearClip, frustum.farClip);
	CalculateView();
}

void Camera::CalculateView()
{
	glm::mat4x4 translation = glm::translate(glm::mat4x4(1.0f), position);

	model = translation * rotation;

	view = glm::inverse(model);

	projectionView = projection * view;
}


OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
{
	frustum.left = left;
	frustum.right = right;
	frustum.bottom = bottom;
	frustum.top = top;
	frustum.nearClip = -1.0f;
	frustum.farClip = 1.0f;

	projection = glm::ortho(left, right, bottom, top);
	CalculateView();
}

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float nearClip, float farClip)
{
	frustum.left = left;
	frustum.right = right;
	frustum.bottom = bottom;
	frustum.top = top;
	frustum.nearClip = nearClip;
	frustum.farClip = farClip;

	projection = glm::ortho(left, right, bottom, top, nearClip, farClip);
	CalculateView();
}


PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip)
{
	frustum.fov = fov;
	frustum.aspectRatio = aspectRatio;
	frustum.nearClip = nearClip;
	frustum.farClip = farClip;

	projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
	CalculateView();
}

} // namespace Hedge
