#include <Application/Application.h>

#include <Message/KeyMessage.h>

#include <Renderer/OpenGLContext.h>
#include <Renderer/DirectX12Context.h>

#include <iostream>


void Application::Run()
{
	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (running)
	{
		auto& previousFrameDuration = frameDuration.GetDuration();

		frameDuration.Start();

		RenderCommand::BeginFrame();

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

		RenderCommand::EndFrame();

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

	case RendererAPI::API::DirectX12:
		renderContext = new DirectX12Context(window.GetHandle());;
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

	RenderCommand::Init(renderContext);

	RenderCommand::SetViewport(window.GetWidth(), window.GetHeight());
	RenderCommand::SetClearColor(clear_color);

	Renderer::SetWireframeMode(false);
	Renderer::SetDepthTest(true);
	Renderer::SetFaceCulling(true);
	Renderer::SetBlending(true);

	imGuiComponent = new ImGuiComponent(window.GetHandle(), renderContext);

	// Show the window
	window.Show();
	window.Update();
}

void Application::OnMessage(Message& message)
{
	if (message.GetMessageType() == MessageType::WindowSize)
	{
		// TODO by doing it this way, we're resizing with every pixel change, maybe not the best way
		const WindowSizeMessage& windowSizeMessage = dynamic_cast<const WindowSizeMessage&>(message);
		RenderCommand::SetViewport(windowSizeMessage.GetWidth(), windowSizeMessage.GetHeight());
		window.SetSize(windowSizeMessage.GetWidth(), windowSizeMessage.GetHeight());

		// TODO for continuously update the window while it's being resized (mouse button is down and dragging)
		// we need to put the updates calls into the WM_SIZE handler because it doesn't return to the main message loop until it's
		// done resizing (mouse button up)
		// The application is effectively stopped during resizing so we don't need to worry about delta time
		// For now just copy paste most of the main application run loop
		if (true)
		{
			RenderCommand::BeginFrame();

			// Fire OnUpdate functions (like rendering) in order, first layers, overlays after
			for (auto layer : layers)
			{
				if (layer->IsEnabled())
				{
					layer->OnUpdate(std::chrono::duration<double, std::milli>(0.0));
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

			RenderCommand::EndFrame();
		}
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
	delete imGuiComponent;

	// you really shouldn't call ->Release on ComPtr objects as it releases automatically - it is a smart pointer, think std::shared_ptr
	// to explicitly release resources through ComPtr, use the Reset() call
	//m_rootSignature.Reset();
	//m_pipelineState.Reset();

	// I think everything touching the d3d12 device needs to be cleaned up before releasing the device itself
	// So clean up everything else before deleting the context
	delete renderContext;
}
