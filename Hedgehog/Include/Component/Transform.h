#pragma once

#include <glm/glm.hpp>


namespace Hedge
{

class Transform
{
public:
	Transform();

	const glm::mat4& Get();
	const glm::vec3& GetTranslation() const { return translation; }
	const glm::vec3& GetRotation() const { return rotation; }
	const glm::vec3& GetScale() const { return scale; }

	Transform operator*(const Transform& t1) const;

	// TODECIDE consider returning a reference to itself from all these transformation functions
	// so they can be chained

	// Set transformation components absolutely
	void SetTranslation(const glm::vec3& translation);
	void SetRotation(const glm::vec3& rotation);
	void SetRotation(const glm::mat4& rotation);
	void SetRotation(const glm::quat& rotation);
	void SetScale(const glm::vec3& scale);
	void SetUniformScale(float scale);

	// Set transformation components relatively
	void Translate(const glm::vec3& translation);
	void Rotate(const glm::vec3& rotation);
	void Scale(const glm::vec3& scale);
	void UniformScale(float scale);

	void TranslateAbsolute(const glm::vec3& translation);
	void RotateAbsolute(const glm::vec3& rotation);
	void ScaleAbsolute(const glm::vec3& scale);
	void UniformScaleAbsolute(float scale);

	void CreateGuiControls(bool controlTranslation = true,
						   bool controlRotation = true,
						   bool controlScale = true);

private:
	bool updateTransform = true;
	glm::mat4 transform = glm::mat4(1.0f);

	glm::vec3 translation = glm::vec3(0.0f);
	// TODO quaternion for rotation
	glm::vec3 rotation = glm::vec3(0.0f); // in degrees
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 translationMatrix;
	glm::mat4 rotationMatrix;
	glm::mat4 scaleMatrix;

	bool GUIuniformScale = true;
};

} // namespace Hedge
