#include <Application/Application.h>

#include <Message/KeyMessage.h>


#include <iostream>


// GLAD completely replaces GL.h
// If there is something missing it means that the glad files were not generated properly,
// for example glBegin, glColor3f, glEnd are in the compatibility profile and not in the core
#include <glad/glad.h>


// GLAD_WGL completely replaces any WGL extensions you would load using wglext.h and something like example for swap_interval below
//bool WGLExtensionSupported(const char* extension_name)
//{
//    // this is pointer to function which returns pointer to string with list of all wgl extensions
//    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;
//
//    // determine pointer to wglGetExtensionsStringEXT function
//    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
//
//    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
//    {
//        // string was not found
//        return false;
//    }
//
//    // extension is supported
//    return true;
//}

//if (WGLExtensionSupported("WGL_EXT_swap_control"))
//{
//    printf("WGL_EXT_swap_control exists\n");
//    // Extension is supported, init pointers.
//    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

//    // this is another function from WGL_EXT_swap_control extension
//    PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");

//    wglSwapIntervalEXT(1);
//}
//
// If there is something missing it means that the glad files were not generated properly,
// for example, WGL_EXT_swap_control has WGL_EXT_extensions_string as a dependency
#include <glad/glad_wgl.h>


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


		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

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

		// Rendering
		ImGui::Render();

		// Test, draw a trinagle on top of the ImGui
		glViewport(0, 0, 1280, 720);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glBegin(GL_TRIANGLES);

		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(0.0f + 0.01f * xOffset, 0.5 + 0.01 * yOffset);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(-0.5f + 0.01f * xOffset, -0.5 + 0.01 * yOffset);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(0.5f + 0.01f * xOffset, -0.5 + 0.01 * yOffset);

		glEnd();

		//wglMakeCurrent(GetDC(window.GetHandle()), context);
		SwapBuffers(GetDC(window.GetHandle()));
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

	// Create OpenGL context
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int pf = ChoosePixelFormat(GetDC(window.GetHandle()), &pfd);
	SetPixelFormat(GetDC(window.GetHandle()), pf, &pfd);

	context = wglCreateContext(GetDC(window.GetHandle()));
	wglMakeCurrent(GetDC(window.GetHandle()), context);


	// Load GL using GLAD's default loader
	if (!gladLoadGL())
	{
		printf("Error gladLoadGL()\n");
	}

	// Load WGL using GLAD's default loader
	if (!gladLoadWGL(GetDC(window.GetHandle())))
	{
		printf("Error gladLoadWGL()\n");
	}

	// Setup VSYNC
	wglSwapIntervalEXT(1);

	// Show the window
	window.Show();
	window.Update();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	////io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	////io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(window.GetHandle());
	ImGui_ImplOpenGL3_Init("#version 460");

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
}

void Application::OnMessage(Message& message)
{
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
	wglDeleteContext(context);
	DeleteDC(GetDC(window.GetHandle()));
}
