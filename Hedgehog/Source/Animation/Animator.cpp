#include <Animation/Animator.h>

#include <imgui.h>

#include <cmath>


namespace Hedge
{

void Animator::Update(float timeStep)
{
	if (running)
	{
		animationTime += timeStep / 1000.0f * animationSpeed;
		animationTime = fmod(animationTime, animation->GetDuration());
	}

	if (running
		|| resetRender)
	{
		transforms = animation->GetTransforms(animationTime);
		resetRender = false;
	}
}

void Animator::CreateGuiControls()
{
	ImGui::PushID(this);

	if (ImGui::Button("Reset"))
	{
		Reset();
	}

	ImGui::SameLine(); if (ImGui::Button("Pause"))
	{
		Pause();
	}

	ImGui::SameLine(); if (ImGui::Button("Resume"))
	{
		Resume();
	}

	ImGui::SliderFloat("Speed", &animationSpeed, 0.1f, 10.0f);

	ImGui::PopID();
}

} // namespace Hedge
