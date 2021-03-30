#pragma once

#include <Component/Transform.h>


namespace Hedge
{

struct Frustum
{
	// Input values, always supplied
	float aspectRatio;

	// Fun Fact: near and far are defined as nothing through windows.h (minwindef.h)
	float nearClip;
	float farClip;
	
	// Input values, camera type specific
	float zoom = 1.0f; // applies only for orthographic camera
	float fov = 56.0f; // applies only for perspective camera
	
	// coordinates of the clip face
	// these values are computed from inputs
	float nearLeft;
	float nearRight;
	float nearBottom;
	float nearTop;
	float farLeft;
	float farRight;
	float farBottom;
	float farTop;
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

	static Camera CreateOrthographic(float aspectRatio, float zoom = 1.0f, float nearClip = 0.0f, float farClip = 1.0f);
	static Camera CreatePerspective(float aspectRatio, float fov = 56.0f, float nearClip = 0.01f, float farClip = 1.0f);

	CameraType GetType() const { return type; }

	void SetProjection(const glm::mat4x4& projection);
	const glm::mat4x4& GetProjection() const { return projection; }

	void SetAspectRatio(float aspectRatio);
	void SetZoom(float zoom);
	void SetFOV(float FOV);

	bool IsPrimary() const { return primary; }
	void SetPrimary(bool primary) { this->primary = primary; }

	const Frustum& GetFrustum() const { return frustum; }

private:
	void CalculateClipFaces();

	void CreateProjectionMatrix();
	void CreateOrthographicMatrix();
	void CreatePerspectiveMatrix();


private:
	CameraType type = CameraType::Unknown;
	glm::mat4x4 projection{1.0f};
	Frustum frustum = Frustum();
	bool primary = true;
};

} // namespace Hedge
