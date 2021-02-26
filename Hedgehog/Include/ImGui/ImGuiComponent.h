#pragma once

#include <imgui.h>

#include <Windows.h>
#include <d3dx12.h>
#include <glad/glad.h>

#include <Renderer/RenderContext.h>


namespace Hedge
{

// TODO should this be just a interface into API specific implementations?
// ImGuiComponent is a wrapper class over ImGui core functions for initialization and rendering
class ImGuiComponent
{
public:
	ImGuiComponent(HWND hwnd, RenderContext* renderContext);
	~ImGuiComponent();

	void BeginFrame();
	void EndFrame();

private:
	void CreateSRVDescHeap();

private:
	RenderContext* renderContext;

	ID3D12DescriptorHeap* SRVDescHeap = nullptr;
};

} // namespace Hedge
