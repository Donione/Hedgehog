#include <Renderer/Camera.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace Hedge
{

void Camera::SetPosition(const glm::vec3& position)
{
	transform.SetTranslation(position);
	CalculateView();
}

void Camera::SetRotation(const glm::vec3& rotation)
{
	transform.SetRotation(rotation);
	CalculateView();
}

void Camera::Move(const glm::vec3& positionOffset)
{
	transform.Translate(positionOffset);
	CalculateView();
}

void Camera::Rotate(const glm::vec3& rotationOffset)
{
	transform.Rotate(rotationOffset);
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
	view = glm::inverse(transform.Get());
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
