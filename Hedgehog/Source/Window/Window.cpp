#include <Window/Window.h>

#include <Windows.h>
#include <windowsx.h>
#include <cstdio>

#include <Message/KeyMessage.h>
#include <Message/MouseMessage.h>
#include <Message/WindowMessage.h>

#include "imgui.h"


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


namespace Hedge
{

LRESULT Window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Wire inputs into the ImGui using the example implementation
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	// Our own handles and other
	switch (uMsg)
	{
	case WM_CREATE:
		printf("%s: Window created.\n", title.c_str());
		break;

	case WM_SIZE:
	{
		// TODO: when the window is being resized by dragging the borders/corner, we're getting a message per pixel of new size
		// maybe we want to just wait for the final new size and don't care about the intermediate sizes
		WindowSizeMessage message(LOWORD(lParam), HIWORD(lParam));

		if (MessageCallback) MessageCallback(message);
		return 0;
	}

	case WM_KEYDOWN:
	{
		unsigned short repeatCount = (unsigned short)(lParam & 0xFFFF);
		bool previousState = ((lParam & (1 << 30)) >> 30);
		KeyPressedMessage message((unsigned char)(wParam & 0xFF), repeatCount, previousState);

		if (MessageCallback) MessageCallback(message);
		return 0;
	}

	case WM_CHAR:
	{
		break;
	}

	case WM_SYSKEYDOWN:
	{
		break;
	}

	case WM_SYSKEYUP:
	{
		break;
	}

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;

	case WM_KEYUP:
	{
		unsigned short repeatCount = (unsigned short)(lParam & 0xFFFF);
		bool previousState = ((lParam & (1 << 30)) >> 30);
		KeyReleasedMessage message((unsigned char)(wParam & 0xFF), repeatCount, previousState);

		if (MessageCallback) MessageCallback(message);
		return 0;
	}

	case WM_MOUSEMOVE:
	{
		MouseMoveMessage message(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		if (MessageCallback) MessageCallback(message);
		return 0;

		break;
	}

	case WM_LBUTTONDBLCLK:
		break;

	case WM_MOUSEWHEEL:
	{
		MouseScrollMessage message(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
		if (MessageCallback) MessageCallback(message);
		return 0;
	}

	case WM_PAINT:
		break;

	case WM_CLOSE:
		printf("%s: Clicked close.\n", title.c_str());
		break;

	case WM_DESTROY:
		printf("%s: Window destroyed.\n", title.c_str());
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window* pThis = NULL;

	if (uMsg == WM_CREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pThis = reinterpret_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
	}
	else
	{
		LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
		pThis = reinterpret_cast<Window*>(ptr);
	}

	if (pThis)
	{
		return pThis->HandleMessage(uMsg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void Window::Create(HINSTANCE hInstance, const WindowProperties windowProperties)
{
	title = windowProperties.title;
	width = windowProperties.width;
	height = windowProperties.height;

	// MessageCallback = ;

	// Fill the window class
	// Note that this is not a C++ "class" but rather an internal structure used by the operating system
	const wchar_t CLASS_NAME[] = L"Hedgehog Window Class";

	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.style |= CS_DBLCLKS;
	wc.style |= CS_OWNDC;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	// Register the window class with the operating system
	RegisterClass(&wc);

	// Window size specified in the call bellow includes borders, headers, menus, etc: the non-client area.
	// We want to set the size of the area that is usable for us: the client area. Following function does that for us.
	RECT clientArea = { 0, 0, (LONG)windowProperties.width, (LONG)windowProperties.height };
	AdjustWindowRectEx(&clientArea,
					   WS_OVERLAPPEDWINDOW,
					   false,
					   0);

	hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		std::wstring(windowProperties.title.begin(), windowProperties.title.end()).c_str(),    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, // X 
		CW_USEDEFAULT, // Y
		clientArea.right - clientArea.left, //windowProperties.width, // Width
		clientArea.bottom - clientArea.top, //windowProperties.height, // Height

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		this        // Additional application data
	);
}

void Window::SetMessageCallback(const std::function<void(Message&)> callback)
{
	MessageCallback = callback;
}

void Window::Show(int nShowCmd)
{
	if (hwnd != NULL)
	{
		ShowWindow(hwnd, nShowCmd);
	}
}

void Window::Update(void)
{
	if (hwnd != NULL)
	{
		UpdateWindow(hwnd);
	}
}

void Window::SetSize(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
}

HWND Window::GetHandle(void)
{
	return hwnd;
}

} // namespace Hedge
