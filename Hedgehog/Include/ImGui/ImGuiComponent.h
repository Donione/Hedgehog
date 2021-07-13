#pragma once

#include <imgui.h>

#include <Windows.h>
#include <d3dx12.h>
#include <glad/glad.h>
#include <vulkan/vulkan.h>

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
	// DirectX12 implementation specific
	void CreateSRVDescHeap();

	// Vulkan implementation specific
	void UploadFonts();

private:
	RenderContext* renderContext;

	ID3D12DescriptorHeap* SRVDescHeap = nullptr;
};

} // namespace Hedge
