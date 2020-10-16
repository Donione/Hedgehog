#include <Application/Application.h>

#include <Message/KeyMessage.h>


#include <iostream>

void Application::Run()
{
    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
    }
}

HWND Application::GetWindowHandle(void)
{
	return window.GetHandle();
}

void Application::Init()
{
	window.Create(hInstance, Window::WindowProperties("Main Window"));

	window.SetMessageCallback(std::bind(&Application::OnMessage, this, std::placeholders::_1));
	//window.SetMessageCallback(&OnMessage); // can't do that
	//window.SetMessageCallback([this] (Message& message) { this->OnMessage(message); }); // this works too

	// Show the window
	window.Show();
	window.Update();
}

void Application::OnMessage(Message& message)
{
	std::cout << message.ToString() << std::endl;
}
