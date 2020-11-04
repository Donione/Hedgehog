#include <Application/Application.h>
#include <Renderer/OpenGLContext.h>

#include <Message/KeyMessage.h>


#include <iostream>


// GLAD completely replaces GL.h
// If there is something missing it means that the glad files were not generated properly,
// for example glBegin, glColor3f, glEnd are in the compatibility profile and not in the core
#include <glad/glad.h>


#include <imgui.h>
#include "imgui_impl_win32.h"
#include <imgui_impl_opengl3.h>


void Application::Run()
{
	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		//printf("Application core: Run loop (OnUpdate) called\n");

		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}


		// Fire OnUpdate functions (like rendering) in order, first layers, overlays after
		for (auto layer : layers)
		{
			if (layer->IsEnabled())
			{
				layer->OnUpdate();
			}
		}


		// Poll WASD input
		if (GetKeyState(0x44) < 0) // 'D'
		{
			xOffset++;
		}

		if (GetKeyState(0x41) < 0) // 'A'
		{
			xOffset--;
		}

		if (GetKeyState(0x57) < 0) // 'W'
		{
			yOffset++;
		}

		if (GetKeyState(0x53) < 0) // 'S'
		{
			yOffset--;
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

		// Note that all of these OnGuiUpdates are actually rendered by calling ImGui::Render() and ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData())
		// which are called in ImGui->EndFrame()
		// So on creen the following immediate GL rendering will be behind the ImGui

		// Triangle
		{
			glBegin(GL_TRIANGLES);

			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex2f(0.0f + 0.01f * xOffset, 0.5f + 0.01f * yOffset);

			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex2f(-0.5f + 0.01f * xOffset, -0.5f + 0.01f * yOffset);

			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex2f(0.5f + 0.01f * xOffset, -0.5f + 0.01f * yOffset);

			glEnd();
		}

		imGuiComponent->EndFrame();

		renderContext->SwapBuffers();
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

	renderContext = new OpenGLContext(window.GetHandle());
	renderContext->Init();

	// Setup VSYNC
	renderContext->SetSwapInterval(1);

	// Show the window
	window.Show();
	window.Update();

	imGuiComponent = new ImGuiComponent(window.GetHandle());
}

void Application::OnMessage(Message& message)
{
	// Fire OnMessage functions in reverse order, first overlays, layers after
	for (auto rit = layers.rbegin(); rit != layers.rend(); ++rit)
	{
		if ((*rit)->IsEnabled())
		{
			(*rit)->OnMessage(message);
		}
	}

	printf("Application core: OnMessage called\n");
	std::cout << message.ToString() << std::endl;

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
}

Application::~Application()
{
	renderContext->Delete();
	delete renderContext;
	delete imGuiComponent;
}
