#pragma once

#include <Windows.h>

#include <string>
#include <functional>


class Window
{
public:
    struct WindowProperties
    {
        std::string title;
        unsigned int width;
        unsigned int height;

        WindowProperties(const std::string& title = "Hedgehog Window",
                         unsigned int width = 1280,
                         unsigned int height = 720)
            :
            title(title),
            width(width),
            height(height)
        {
        }
    };

public:
    Window() : hwnd(NULL) {}

	void Create(HINSTANCE hInstance, const WindowProperties windowProperties = WindowProperties());
    void SetMessageCallback(const std::function<void(void)> callback);
	void Show(int nShowCmd = SW_SHOW);

private:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HWND hwnd = NULL;

    std::string title = "";
    unsigned int width = 0;
    unsigned int height = 0;

    std::function<void(void)> MessageCallback = NULL;
};


// Example of window class with state taken from https://docs.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-
class BaseWindow
{
public:
    BaseWindow() : m_hwnd(NULL) { }

    BOOL Create(
        PCWSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        HWND hWndParent = 0,
        HMENU hMenu = 0
    )
    {
        WNDCLASS wc = { 0 };

        wc.lpfnWndProc = BaseWindow::WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = ClassName();

        RegisterClass(&wc);

        m_hwnd = CreateWindowEx(
            dwExStyle, ClassName(), lpWindowName, dwStyle, x, y,
            nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
        );

        return (m_hwnd ? TRUE : FALSE);
    }

    HWND Window() const { return m_hwnd; }

public:
    PCWSTR  ClassName() const { return L"Sample Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            HandleStuff();
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(m_hwnd, &ps);
        }
        return 0;

        default:
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
        }
        return TRUE;
    }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        BaseWindow* pThis = NULL;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (BaseWindow*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (BaseWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

    void HandleStuff() {}

private:
    HWND m_hwnd;
};