#pragma once

#include <Window/Window.h>
#include <Layer/LayerStack.h>
#include <Renderer/RenderContext.h>
#include <Renderer/Renderer.h>
#include <ImGui/ImGuiComponent.h>
#include <Message/KeyMessage.h>
#include <Message/MouseMessage.h>

#include <assert.h>


class Application
{
public:
	void Run();

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

	HWND GetWindowHandle(void);

private:
	void Init();
	void OnMessage(Message& message);

protected:
	LayerStack layers;

private:
	inline static Application* instance = nullptr;

	HINSTANCE hInstance = nullptr;
	Window window;

	RenderContext* renderContext;

	// ImGuiComponent is an integral part of the application.
	// It is supposed to handle all Gui rendering, which is submitted by layers.
	ImGuiComponent* imGuiComponent;
};
