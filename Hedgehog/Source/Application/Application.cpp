#include <Application/Application.h>

#include <Message/KeyMessage.h>

#include <Renderer/OpenGLContext.h>
#include <Renderer/DirectX12Context.h>
#include <Renderer/VulkanContext.h>

#include <iostream>


namespace Hedge
{

void Application::Run()
{
	RenderCommand::Begin();

	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (running)
	{
		auto& previousFrameDuration = frameDuration.GetDuration();

		frameDuration.Start();

		if (!minimized)
		{
			RenderCommand::BeginFrame();

			// Fire OnUpdate functions (like rendering) in order, first layers, overlays after
			for (auto& layer : layers)
			{
				if (layer->IsEnabled())
				{
					layer->OnUpdate(previousFrameDuration);
				}
			}

			imGuiComponent->BeginFrame();
			// Fire OnGuiUpdate functions in order, first layers, overlays after
			for (auto& layer : layers)
			{
				if (layer->IsEnabled())
				{
					layer->OnGuiUpdate();
				}
			}
			imGuiComponent->EndFrame();

			RenderCommand::EndFrame();
		}

		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				// TODO At this point the window has been destroyed
				// It seems that Vulkan has a problem with cleaning up a device/surface when the associated window is gone
				// We need to catch WM_CLOSE message, finish rendering in progress, clean up all Vulkan objects (like vertex arrays)
				// and only then we can clean up the Vulkan Context and let the window close and be destoryed
				running = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		frameDuration.Stop();
	}

	RenderCommand::End();
}

void Application::Init()
{
	window.Create(hInstance, Window::WindowProperties("Main Window"));

	window.SetMessageCallback(std::bind(&Application::OnMessage, this, std::placeholders::_1));
	//window.SetMessageCallback(&OnMessage); // can't do that
	//window.SetMessageCallback([this] (Message& message) { this->OnMessage(message); }); // this works too

	RenderCommand::Create();

	// TODO Clean up the renderer settings setting
	// We can't call these for OpenGL before creating the context
	// For DirectX 12 it doesn't matter
	// Vulkan uses these setting in context creation
	if (Renderer::GetAPI() == RendererAPI::API::Vulkan)
	{
		Renderer::SetWireframeMode(wireframeMode);
		Renderer::SetDepthTest(depthTest);
		Renderer::SetFaceCulling(faceCulling);
		Renderer::SetBlending(blending);
	}

	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		renderContext = new OpenGLContext(window.GetHandle());
		break;

	case RendererAPI::API::DirectX12:
		renderContext = new DirectX12Context(window.GetHandle());
		break;

	case RendererAPI::API::Vulkan:
		renderContext = new VulkanContext(window.GetHandle());
		break;

	case RendererAPI::API::None:
		renderContext = nullptr;
		break;

	default:
		renderContext = nullptr;
		break;
	}

	assert(renderContext);

	// Setup VSYNC
	renderContext->SetSwapInterval(1);

	if (Renderer::GetAPI() != RendererAPI::API::Vulkan)
	{
		Renderer::SetWireframeMode(wireframeMode);
		Renderer::SetDepthTest(depthTest);
		Renderer::SetFaceCulling(faceCulling);
		Renderer::SetBlending(blending);
	}

	RenderCommand::Init(renderContext);
	RenderCommand::SetViewport(0, 0, window.GetWidth(), window.GetHeight());
	RenderCommand::SetScissor(0, 0, window.GetWidth(), window.GetHeight());
	RenderCommand::SetClearColor(clear_color);

	imGuiComponent = new ImGuiComponent(window.GetHandle(), renderContext);

	// Show the window
	window.Show();
	window.Update();
}

void Application::OnMessage(Message& message)
{
	if (message.GetMessageType() == MessageType::WindowSize)
	{
		if (!initialized)
		{
			initialized = true;
			return;
		}

		// TODO by doing it this way, we're resizing with every pixel change, maybe not the best way
		const WindowSizeMessage& windowSizeMessage = dynamic_cast<const WindowSizeMessage&>(message);
		unsigned int width = windowSizeMessage.GetWidth();
		unsigned int height = windowSizeMessage.GetHeight();

		if (minimized = (width == 0 || height == 0))
		{
			return;
		}

		RenderCommand::Resize(windowSizeMessage.GetWidth(), windowSizeMessage.GetHeight(), true);
		window.SetSize(windowSizeMessage.GetWidth(), windowSizeMessage.GetHeight());

		// TODO for continuously update the window while it's being resized (mouse button is down and dragging)
		// we need to put the updates calls into the WM_SIZE handler because it doesn't return to the main message loop until it's
		// done resizing (mouse button up)
		// The application is effectively stopped during resizing so we don't need to worry about delta time
		// For now just copy paste most of the main application run loop
		// TODO Disable for now because we don't have a good way to update Vulkan pipeline with the new viewport
		if (false)
		{
			RenderCommand::BeginFrame();

			// Fire OnUpdate functions (like rendering) in order, first layers, overlays after
			for (auto& layer : layers)
			{
				if (layer->IsEnabled())
				{
					layer->OnUpdate(std::chrono::duration<double, std::milli>(0.0));
				}
			}

			imGuiComponent->BeginFrame();
			// Fire OnGuiUpdate functions in order, first layers, overlays after
			for (auto& layer : layers)
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

} // namespace Hedge
