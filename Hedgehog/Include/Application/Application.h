#pragma once

#pragma comment(lib, "D3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")

#pragma comment(lib, "vulkan-1.lib")

#include <Window/Window.h>
#include <Layer/LayerStack.h>
#include <Renderer/RenderContext.h>
#include <Renderer/Renderer.h>
#include <ImGui/ImGuiComponent.h>
#include <Message/KeyMessage.h>
#include <Message/MouseMessage.h>
#include <Message/WindowMessage.h>
#include <Utilities/Stopwatch.h>

#include <assert.h>


namespace Hedge
{

class Application
{
public:
	void Run();

	static Application& GetInstance() { return *instance; }

	RenderContext* GetRenderContext() { return renderContext; }
	Window& GetWindow() { return window; }
	const HINSTANCE GetHInstance() const { return hInstance; }

protected:
	Application(HINSTANCE hInstance)
	{
		// There is supposed to be only one application
		assert(instance == nullptr);

		instance = this;

		this->hInstance = hInstance;
		Init();
	}

	~Application();

	const HWND GetWindowHandle(void) const { return window.GetHandle(); }

private:
	void Init();
	void OnMessage(Message& message);

protected:
	LayerStack layers;

private:
	inline static Application* instance = nullptr;

	bool running = true;

	HINSTANCE hInstance = nullptr;
	Window window;

	RenderContext* renderContext;

	glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

	// ImGuiComponent is an integral part of the application.
	// It is supposed to handle all Gui rendering, which is submitted by layers.
	ImGuiComponent* imGuiComponent;

	Stopwatch frameDuration;

	bool initialized = false;
};

} // namespace Hedge
