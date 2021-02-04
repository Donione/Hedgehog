#pragma once

#include <Windows.h>

#include <imgui.h>

#include <Renderer/RenderContext.h>

// ImGuiComponent is a wrapper class over ImGui core functions for initialization and rendering
class ImGuiComponent
{
public:
	ImGuiComponent(HWND hwnd, RenderContext* renderContext);
	~ImGuiComponent();

	void BeginFrame();
	void EndFrame();

private:
	RenderContext* renderContext;
};