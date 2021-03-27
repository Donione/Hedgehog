#include <Component/Light.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>


namespace Hedge
{

void DirectionalLight::CreateGuiControls()
{
	ImGui::PushID(this);

	ImGui::ColorEdit3("Color", glm::value_ptr(color));
	ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.01f, -10.0, 10.0f);

	ImGui::PopID();
}

bool PointLight::CreateGuiControls()
{
	bool valueChanged = false;

	ImGui::PushID(this);

	ImGui::ColorEdit3("Color", glm::value_ptr(color));
	valueChanged = ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f, -20.0f, 20.0f);

	ImGui::BeginGroup();
	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

	ImGui::DragFloat("##x", &attenuation.x, 0.1f, 0.0f, 10.f); ImGui::SameLine(0.0, 4.0);
	ImGui::PopItemWidth();

	ImGui::DragFloat("##y", &attenuation.y, 0.01f, 0.0f, 1.f); ImGui::SameLine(0.0, 4.0);
	ImGui::PopItemWidth();

	ImGui::DragFloat("Attenuation##z", &attenuation.z, 0.001f, 0.0f, 0.1f);
	ImGui::PopItemWidth();

	ImGui::EndGroup();

	ImGui::PopID();

	return valueChanged;
}

bool SpotLight::CreateGuiControls()
{
	bool valueChanged = false;

	ImGui::PushID(this);

	ImGui::ColorEdit3("Color", glm::value_ptr(color));
	valueChanged |= ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f, -20.0f, 20.0f);
	valueChanged |= ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.01f, -10.0, 10.0f);

	auto tempCutoffAngle = glm::degrees(glm::acos(cutoffAngle));
	ImGui::BeginGroup();
	ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
	ImGui::DragFloat("##inner", &tempCutoffAngle.x, 0.1f, tempCutoffAngle.y - 20.0f, tempCutoffAngle.y); ImGui::SameLine(0.0, 4.0);
	ImGui::PopItemWidth();
	ImGui::DragFloat("Inner/Outer cutoff angle", &tempCutoffAngle.y, 0.1f, tempCutoffAngle.x, 180.0f);
	ImGui::PopItemWidth();
	ImGui::EndGroup();
	cutoffAngle = glm::cos(glm::radians(tempCutoffAngle));

	ImGui::BeginGroup();
	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

	ImGui::DragFloat("##x", &attenuation.x, 0.1f, 0.0f, 10.f); ImGui::SameLine(0.0, 4.0);
	ImGui::PopItemWidth();

	ImGui::DragFloat("##y", &attenuation.y, 0.01f, 0.0f, 1.f); ImGui::SameLine(0.0, 4.0);
	ImGui::PopItemWidth();

	ImGui::DragFloat("Attenuation##z", &attenuation.z, 0.001f, 0.0f, 0.1f);
	ImGui::PopItemWidth();

	ImGui::EndGroup();

	ImGui::PopID();

	return valueChanged;
}

} // namespace Hedge
