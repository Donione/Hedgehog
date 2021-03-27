#include <Component/Transform.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>


namespace Hedge
{

glm::mat4 CreateTranslationMatrix(const glm::vec3& translation)
{
	return glm::translate(glm::mat4(1.0f), translation);
}

glm::mat4 CreateRotationMatrix(const glm::vec3& rotation)
{
	return (glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)));
}

glm::mat4 CreateScaleMatrix(const glm::vec3& scale)
{
	return glm::scale(glm::mat4(1.0f), scale);
}

Transform::Transform()
{
	translationMatrix = CreateTranslationMatrix(translation);
	rotationMatrix = CreateRotationMatrix(rotation);
	scaleMatrix = CreateScaleMatrix(scale);
}

const glm::mat4& Transform::Get()
{
	if (updateTransform)
	{
		transform = translationMatrix * rotationMatrix * scaleMatrix;

		updateTransform = false;
	}

	return transform;
}

void Transform::SetTranslation(const glm::vec3& translation)
{
	this->translation = translation;
	translationMatrix = CreateTranslationMatrix(this->translation);
	updateTransform = true;
}

void Transform::Translate(const glm::vec3& translation)
{
	glm::vec4 orientedTranslation = rotationMatrix * glm::vec4(translation, 1.0f);
	SetTranslation(this->translation + glm::vec3(orientedTranslation));
}

void Transform::TranslateAbsolute(const glm::vec3& translation)
{
	SetTranslation(this->translation + translation);
}

void Transform::SetRotation(const glm::vec3& rotation)
{
	this->rotation = rotation;
	rotationMatrix = CreateRotationMatrix(this->rotation);
	updateTransform = true;
}

void Transform::SetRotation(const glm::mat4& rotation)
{
	rotationMatrix = rotation;

	glm::extractEulerAngleXYZ(rotationMatrix, this->rotation.x, this->rotation.y, this->rotation.z);
	this->rotation = glm::degrees(this->rotation);
	updateTransform = true;
}

void Transform::Rotate(const glm::vec3& rotation)
{
	rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::extractEulerAngleXYZ(rotationMatrix, this->rotation.x, this->rotation.y, this->rotation.z);
	this->rotation = glm::degrees(this->rotation);
	updateTransform = true;
}

void Transform::RotateAbsolute(const glm::vec3& rotation)
{
	SetRotation(this->rotation + rotation);
}

void Transform::SetScale(const glm::vec3& scale)
{
	this->scale = scale;
	scaleMatrix = CreateScaleMatrix(this->scale);
	updateTransform = true;
}

void Transform::SetUniformScale(float scale)
{
	SetScale(glm::vec3(scale, scale, scale));
}

void Transform::Scale(const glm::vec3& scale)
{
	SetScale(this->scale * scale);
}

void Transform::ScaleAbsolute(const glm::vec3& scale)
{
	SetScale(this->scale + scale);
}

void Transform::UniformScale(float scale)
{
	Scale(glm::vec3(scale, scale, scale));
}

void Transform::UniformScaleAbsolute(float scale)
{
	SetScale(this->scale + glm::vec3(scale, scale, scale));
}

void Transform::CreateGuiControls(bool controlTranslation,
								  bool controlRotation,
								  bool controlScale)
{
	ImGui::PushID(this);
	if (controlTranslation)
	{
		auto tempTranslation = translation;
		if (ImGui::SliderFloat3("translate", glm::value_ptr(tempTranslation), -30.0f, 30.0f))
		{
			SetTranslation(tempTranslation);
		}
	}

	if (controlRotation)
	{
		auto tempRotation = rotation;
		if (ImGui::SliderFloat3("rotate", glm::value_ptr(tempRotation), -360.0, 360.0))
		{
			SetRotation(tempRotation);
		}
	}

	if (controlScale)
	{
		auto tempScale = scale;
		bool valueChanged = false;
		if (GUIuniformScale)
		{
			valueChanged = ImGui::SliderFloat("scale", &tempScale.x, 0.0f, 10.0f);
			tempScale.y = tempScale.z = tempScale.x;
		}
		else
		{
			valueChanged = ImGui::SliderFloat3("scale", glm::value_ptr(tempScale), 0.0f, 10.0f);
		}
		ImGui::SameLine();
		ImGui::Checkbox("Uniform", &GUIuniformScale);
		if (valueChanged)
		{
			SetScale(tempScale);
		}
	}
	ImGui::PopID();
}

} // namespace Hedge
