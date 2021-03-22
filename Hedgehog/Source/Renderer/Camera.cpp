#include <Renderer/Camera.h>

#include <Renderer/Renderer.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace Hedge
{

Camera Camera::CreateOrthographic(float left, float right, float bottom, float top, float nearClip, float farClip)
{
	Camera camera;

	camera.type = CameraType::Orthographic;

	camera.frustum.left = left;
	camera.frustum.right = right;
	camera.frustum.bottom = bottom;
	camera.frustum.top = top;
	camera.frustum.nearClip = nearClip;
	camera.frustum.farClip = farClip;

	camera.frustum.zoom = 1.0f;
	camera.frustum.aspectRatio = right / top;

	camera.projection = camera.CreateOrthographicMatrix();

	return camera;
}

Camera Camera::CreatePerspective(float fov, float aspectRatio, float nearClip, float farClip)
{
	Camera camera;

	camera.type = CameraType::Perspective;

	camera.frustum.fov = fov;
	camera.frustum.aspectRatio = aspectRatio;
	camera.frustum.nearClip = nearClip;
	camera.frustum.farClip = farClip;

	camera.projection = camera.CreatePerspectiveMatrix();

	return camera;
}

void Camera::SetProjection(const glm::mat4x4& projection)
{
	// If the projection is set directly, it is treated as a custom projection that could be anything
	// So functions like SetAspectRatio do nothing
	this->type = CameraType::Custom;
	this->projection = projection;
}

void Camera::SetAspectRatio(float aspectRatio)
{
	if (type == CameraType::Orthographic)
	{
		frustum.left = -aspectRatio;
		frustum.right = aspectRatio;;

		frustum.aspectRatio = aspectRatio;

		projection = CreateOrthographicMatrix();
	}
	else if (type == CameraType::Perspective)
	{
		frustum.aspectRatio = aspectRatio;

		projection = CreatePerspectiveMatrix();
	}
}

void Camera::SetZoom(float zoom)
{
	if (type == CameraType::Orthographic)
	{
		frustum.zoom = zoom;

		projection = CreateOrthographicMatrix();
	}
}


void Camera::SetFOV(float FOV)
{
	if (type == CameraType::Perspective)
	{
		frustum.fov = FOV;

		projection = CreatePerspectiveMatrix();
	}
}

glm::mat4 Camera::CreateOrthographicMatrix()
{
	if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
	{
		// DirectX clip volume z normalized device coordinates go from 0 to 1
		return glm::orthoRH_ZO(frustum.left * frustum.zoom,
							   frustum.right * frustum.zoom,
							   frustum.bottom * frustum.zoom,
							   frustum.top * frustum.zoom,
							   frustum.nearClip,
							   frustum.farClip);
	}
	else
	{
		// OpenGL clip volume z normalized device coordinate go from -1 to 1
		return glm::ortho(frustum.left * frustum.zoom,
						  frustum.right * frustum.zoom,
						  frustum.bottom * frustum.zoom,
						  frustum.top * frustum.zoom,
						  frustum.nearClip,
						  frustum.farClip);
	}
}

glm::mat4 Camera::CreatePerspectiveMatrix()
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

} // namespace Hedge
