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


		if (GetKeyState(0x44) < 0)
		{
			xOffset++;
		}

		if (GetKeyState(0x41) < 0)
		{
			xOffset--;
		}

		if (GetKeyState(0x57) < 0)
		{
			yOffset++;
		}

		if (GetKeyState(0x53) < 0)
		{
			yOffset--;
		}


		imGuiComponent->BeginFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("+"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			if (ImGui::Button("-"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter--;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Note that all of these windows are actually rendered by calling ImGui::Render() and ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData())
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
		//case 0x44: // 'D'
		//	xOffset++; break;

		//case 0x41: // 'A'
		//	xOffset--; break;

		//case 0x57: // 'W'
		//	yOffset++; break;

		//case 0x53: // 'S'
		//	yOffset--; break;

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
