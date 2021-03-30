#include <Renderer/Camera.h>

#include <Renderer/Renderer.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace Hedge
{

Camera Camera::CreateOrthographic(float aspectRatio, float zoom, float nearClip, float farClip)
{
	Camera camera;

	camera.type = CameraType::Orthographic;

	camera.frustum.aspectRatio = aspectRatio;
	camera.frustum.nearClip = nearClip;
	camera.frustum.farClip = farClip;
	camera.frustum.zoom = zoom;

	camera.CalculateClipFaces();
	camera.CreateOrthographicMatrix();

	return camera;
}

Camera Camera::CreatePerspective(float aspectRatio, float fov, float nearClip, float farClip)
{
	Camera camera;

	camera.type = CameraType::Perspective;

	camera.frustum.aspectRatio = aspectRatio;
	camera.frustum.nearClip = nearClip;
	camera.frustum.farClip = farClip;
	camera.frustum.fov = fov;

	camera.CalculateClipFaces();
	camera.CreatePerspectiveMatrix();

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
	if (type == CameraType::Orthographic
		|| type == CameraType::Perspective)
	{
		frustum.aspectRatio = aspectRatio;

		CalculateClipFaces();
		CreateProjectionMatrix();
	}
}

void Camera::SetZoom(float zoom)
{
	if (type == CameraType::Orthographic)
	{
		frustum.zoom = zoom;

		CalculateClipFaces();
		CreateOrthographicMatrix();
	}
}

void Camera::SetFOV(float FOV)
{
	if (type == CameraType::Perspective)
	{
		frustum.fov = FOV;

		CalculateClipFaces();
		CreatePerspectiveMatrix();
	}
}

void Camera::CalculateClipFaces()
{
	if (type == CameraType::Orthographic)
	{
		// near and far face both have the same x and y coordinates
		frustum.farLeft = frustum.nearLeft = -frustum.aspectRatio;
		frustum.farRight = frustum.nearRight = frustum.aspectRatio;
		frustum.farBottom = frustum.nearBottom = -1.0;
		frustum.farTop = frustum.nearTop = 1.0;
	}
	else if(type == Hedge::CameraType::Perspective)
	{
		frustum.nearTop = glm::tan(glm::radians(frustum.fov / 2.0f)) * frustum.nearClip;
		frustum.nearBottom = -frustum.nearTop;
		frustum.nearRight = frustum.aspectRatio * frustum.nearTop;
		frustum.nearLeft = -frustum.nearRight;
		frustum.farTop = glm::tan(glm::radians(frustum.fov / 2.0f)) * frustum.farClip;
		frustum.farBottom = -frustum.farTop;
		frustum.farRight = frustum.aspectRatio * frustum.farTop;
		frustum.farLeft = -frustum.farRight;
	}
}

void Camera::CreateProjectionMatrix()
{
	if (type == CameraType::Orthographic)
	{
		CreateOrthographicMatrix();
	}
	else
	{
		CreatePerspectiveMatrix();
	}
}

void Camera::CreateOrthographicMatrix()
{
	if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
	{
		// DirectX clip volume z normalized device coordinates go from 0 to 1
		projection = glm::orthoRH_ZO(frustum.nearLeft * frustum.zoom,
									 frustum.nearRight * frustum.zoom,
									 frustum.nearBottom * frustum.zoom,
									 frustum.nearTop * frustum.zoom,
									 frustum.nearClip,
									 frustum.farClip);
	}
	else
	{
		// OpenGL clip volume z normalized device coordinate go from -1 to 1
		projection = glm::ortho(frustum.nearLeft * frustum.zoom,
								frustum.nearRight * frustum.zoom,
								frustum.nearBottom * frustum.zoom,
								frustum.nearTop * frustum.zoom,
								frustum.nearClip,
								frustum.farClip);
	}
}

void Camera::CreatePerspectiveMatrix()
{
	if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
	{
		projection = glm::perspectiveRH_ZO(glm::radians(frustum.fov), frustum.aspectRatio, frustum.nearClip, frustum.farClip);
	}
	else
	{
		projection = glm::perspective(glm::radians(frustum.fov), frustum.aspectRatio, frustum.nearClip, frustum.farClip);
	}
}

} // namespace Hedge
