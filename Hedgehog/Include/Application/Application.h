#pragma once

#include <Window/Window.h>
#include <Renderer/RenderContext.h>

#include <assert.h>

#include <imgui.h>

class Application
{
public:
	Application(HINSTANCE hInstance)
	{
		// There is supposed to be only one application
		assert(instance == nullptr);

		instance = this;

		this->hInstance = hInstance;
		Init();
	}

	~Application();

	void Run();

	HWND GetWindowHandle(void);

private:
	void Init();
	void OnMessage(Message& message);

private:
	inline static Application* instance;

	HINSTANCE hInstance = nullptr;
	Window window;

	RenderContext* renderContext;

	// Our state
	int counter = 1;
	int xOffset = 0;
	int yOffset = 0;
	bool show_demo_window = false;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};
