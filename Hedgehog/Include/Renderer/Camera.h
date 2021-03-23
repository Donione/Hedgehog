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

	// zoom applies only for orthographic camera
	float zoom;

	// fov applies only for perspective camera
	float fov;
	float aspectRatio;

	Frustum() = default;
};

enum class CameraType
{
	Unknown,
	Orthographic,
	Perspective,
	Custom,
};

class Camera
{
public:
	Camera() = default;

	static Camera CreateOrthographic(float left, float right, float bottom, float top, float nearClip = 0.0f, float farClip = 1.0f);
	static Camera CreatePerspective(float fov, float aspectRatio, float nearClip = 0.01f, float farClip = 1.0f);


	CameraType GetType() const { return type; }

	void SetProjection(const glm::mat4x4& projection);
	const glm::mat4x4& GetProjection() const { return projection; }

	void SetAspectRatio(float aspectRatio);
	void SetZoom(float zoom);
	void SetFOV(float FOV);

private:
	glm::mat4 CreateOrthographicMatrix();
	glm::mat4 CreatePerspectiveMatrix();


private:
	CameraType type = CameraType::Unknown;
	glm::mat4x4 projection{1.0f};
	Frustum frustum = Frustum();
};

} // namespace Hedge
