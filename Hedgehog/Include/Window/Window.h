#pragma once

#include <Windows.h>
#include <Message/Message.h>

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
    void SetMessageCallback(const std::function<void(Message&)> callback);
	void Show(int nShowCmd = SW_SHOW);
    void Update(void);

    HWND GetHandle(void);

private:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HWND hwnd = NULL;

    std::string title = "";
    unsigned int width = 0;
    unsigned int height = 0;

    std::function<void(Message&)> MessageCallback = NULL;
};
