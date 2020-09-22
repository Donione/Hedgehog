#include <cstdlib>
#include <cstdio>
#include <functional>
#include <iostream>

#include <Window/Window.h>
#include <Message/Message.h>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
//#include <spdlog/spdlog.h>
//#include "spdlog/sinks/stdout_color_sinks.h"

#include <Windows.h>


class Application
{
public:
	void foo(Message& message)
	{
		std::cout << message.ToString() << std::endl;
	}
};


int WINAPI wWinMain(_In_ HINSTANCE hInstance,
					_In_opt_ HINSTANCE hPrevInstance,
					_In_ LPWSTR lpCmdLine,
					_In_ int nShowCmd)
{
	FILE* fp;

	AllocConsole();
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);

	Window window;
	Application app;
	window.Create(hInstance, Window::WindowProperties("Main Window"));
	window.SetMessageCallback(std::bind(&Application::foo, &app, std::placeholders::_1));
	//window.SetMessageCallback(&app.foo); // can't do that
	//window.SetMessageCallback([&app] () { app.foo(); }); // this works too
	window.Show();

	// Run the message loop
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return EXIT_SUCCESS;
}

