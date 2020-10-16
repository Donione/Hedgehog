#include <Window/Window.h>

#include <Windows.h>
#include <cstdio>

#include <Message/KeyMessage.h>


LRESULT Window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        printf("%s: Window created.\n", title.c_str());
        break;

    case WM_CLOSE:
        printf("%s: Clicked close.\n", title.c_str());
        break;

    case WM_DESTROY:
        printf("%s: Window destroyed.\n", title.c_str());
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
    {
        unsigned short repeatCount = (unsigned short)(lParam & 0xFFFF);
        bool previousState = ((lParam & (1 << 30)) >> 30);
        KeyPressedMessage message(wParam, repeatCount, previousState);

        if (MessageCallback) MessageCallback(message);
        break;
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

    case WM_KEYUP:
    {
        unsigned short repeatCount = (unsigned short)(lParam & 0xFFFF);
        bool previousState = ((lParam & (1 << 30)) >> 30);
        KeyReleasedMessage message(wParam, repeatCount, previousState);

        if (MessageCallback) MessageCallback(message);
        break;
    }

    case WM_MOUSEMOVE:
        break;

    case WM_LBUTTONDBLCLK:
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
        return 0;
    }

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
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	// Register the window class with the operating system
	RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        std::wstring(windowProperties.title.begin(), windowProperties.title.end()).c_str(),    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, // X 
        CW_USEDEFAULT, // Y
        windowProperties.width, // Width
        windowProperties.height, // Height

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
