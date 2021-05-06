#pragma once

#include <Animation/Animation.h>

#include <memory>


namespace Hedge
{

class Animator
{
public:
	Animator(Animation* animation) : animation(animation) {}

	void Update(float timeStep);

	const std::vector<glm::mat4>& GetTransforms() const { return transforms; }

	void Reset() { animationTime = 0.0f; resetRender = true; }
	void Pause() { running = false; }
	void Resume() { running = true; }

	void CreateGuiControls();

private:
	Animation* animation;

	float animationSpeed = 1.0f;
	bool running = true;
	bool resetRender = false;
	float animationTime = 0.0f;

	std::vector<glm::mat4> transforms;
};

} // namespace Hedge
