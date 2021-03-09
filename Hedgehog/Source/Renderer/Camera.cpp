#include <Renderer/Camera.h>

#include <Renderer/Renderer.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace Hedge
{

glm::mat4 OrthographicCamera::CreateOrthographicMatrix()
{
	if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
	{
		// DirectX clip volume z normalized device coordinates go from 0 to 1
		return glm::orthoRH_ZO(frustum.left * zoom, frustum.right * zoom, frustum.bottom * zoom, frustum.top * zoom, frustum.nearClip, frustum.farClip);
	}
	else
	{
		// OpenGL clip volume z normalized device coordinate go from -1 to 1
		return glm::ortho(frustum.left * zoom, frustum.right * zoom, frustum.bottom * zoom, frustum.top * zoom, frustum.nearClip, frustum.farClip);
	}
}

glm::mat4 PerspectiveCamera::CreatePerspectiveMatrix()
{
	if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
	{
		return glm::perspectiveRH_ZO(glm::radians(frustum.fov), frustum.aspectRatio, frustum.nearClip, frustum.farClip);
	}
	else
	{
		return glm::perspective(glm::radians(frustum.fov), frustum.aspectRatio, frustum.nearClip, frustum.farClip);
	}
}

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

void Camera::CalculateView()
{
	view = glm::inverse(transform.Get());
	projectionView = projection * view;
}


OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float nearClip, float farClip)
{
	frustum.left = left;
	frustum.right = right;
	frustum.bottom = bottom;
	frustum.top = top;
	frustum.nearClip = nearClip;
	frustum.farClip = farClip;

	frustum.aspectRatio = right / top;

	projection = CreateOrthographicMatrix();
	CalculateView();
}

void OrthographicCamera::SetAspectRatio(float aspectRatio)
{
	frustum.left = -aspectRatio;
	frustum.right = aspectRatio;;

	frustum.aspectRatio = aspectRatio;

	projection = CreateOrthographicMatrix();
	CalculateView();
}

void OrthographicCamera::SetZoom(float zoom)
{
	this->zoom = zoom;

	projection = CreateOrthographicMatrix();
	CalculateView();
}


PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip)
{
	frustum.fov = fov;
	frustum.aspectRatio = aspectRatio;
	frustum.nearClip = nearClip;
	frustum.farClip = farClip;

	projection = CreatePerspectiveMatrix();
	CalculateView();
}

void PerspectiveCamera::SetAspectRatio(float aspectRatio)
{
	frustum.aspectRatio = aspectRatio;

	projection = CreatePerspectiveMatrix();
	CalculateView();
}

void PerspectiveCamera::SetFOV(float FOV)
{
	frustum.fov = FOV;

	projection = CreatePerspectiveMatrix();
	CalculateView();
}

} // namespace Hedge
