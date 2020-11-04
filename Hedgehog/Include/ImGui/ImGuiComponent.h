#pragma once

#include <Windows.h>

// ImGuiComponent is a wrapper class over ImGui core functions for initialization and rendering
class ImGuiComponent
{
public:
	ImGuiComponent(HWND hwnd);
	~ImGuiComponent();

	void BeginFrame();
	void EndFrame();
};