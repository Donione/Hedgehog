#pragma once

#include <Window/Window.h>

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

	HGLRC context = NULL;

	// Our state
	int counter = 1;
	bool show_demo_window = false;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};
