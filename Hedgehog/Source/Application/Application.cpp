#include <Application/Application.h>

#include <Message/KeyMessage.h>

#include <Renderer/OpenGLContext.h>

#include <iostream>


void Application::Run()
{
	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (running)
	{
		printf("Application core: Run loop (OnUpdate) called\n");

		auto& previousFrameDuration = frameDuration.GetDuration();

		frameDuration.Start();

		RenderCommand::Clear();

		// Fire OnUpdate functions (like rendering) in order, first layers, overlays after
		for (auto layer : layers)
		{
			if (layer->IsEnabled())
			{
				layer->OnUpdate(previousFrameDuration);
			}
		}

		imGuiComponent->BeginFrame();
		// Fire OnGuiUpdate functions in order, first layers, overlays after
		for (auto layer : layers)
		{
			if (layer->IsEnabled())
			{
				layer->OnGuiUpdate();
			}
		}
		imGuiComponent->EndFrame();

		renderContext->SwapBuffers();

		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				running = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		frameDuration.Stop();
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

	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		renderContext = new OpenGLContext(window.GetHandle());
		break;

	case RendererAPI::API::None:
		renderContext = nullptr;
		break;

	default:
		renderContext = nullptr;
		break;
	}

	// Setup VSYNC
	renderContext->SetSwapInterval(0);

	Renderer::SetWireframeMode(false);
	Renderer::SetDepthTest(true);
	Renderer::SetFaceCulling(true);
	Renderer::SetBlending(true);

	// Show the window
	window.Show();
	window.Update();

	imGuiComponent = new ImGuiComponent(window.GetHandle());
}

void Application::OnMessage(Message& message)
{
	if (message.GetMessageType() == MessageType::WindowSize)
	{
		const WindowSizeMessage& windowSizeMessage = dynamic_cast<const WindowSizeMessage&>(message);
		RenderCommand::SetViewport(windowSizeMessage.GetWidth(), windowSizeMessage.GetHeight());
		window.SetSize(windowSizeMessage.GetWidth(), windowSizeMessage.GetHeight());
	}

	if (message.GetMessageType() == MessageType::KeyPressed)
	{
		KeyPressedMessage& keyPressedMessage = dynamic_cast<KeyPressedMessage&>(message);

		switch (keyPressedMessage.GetKeyCode())
		{
		case VK_ESCAPE:
			::PostMessage(window.GetHandle(), WM_CLOSE, 0, 0); break;

		default: break;
		}
	}

	// Fire OnMessage functions in reverse order, first overlays, layers after
	for (auto rit = layers.rbegin(); rit != layers.rend(); ++rit)
	{
		if ((*rit)->IsEnabled())
		{
			(*rit)->OnMessage(message);
		}
	}
}

Application::~Application()
{
	delete renderContext;
	delete imGuiComponent;
}
