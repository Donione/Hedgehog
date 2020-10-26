#pragma once

#include <Renderer/RenderContext.h>

#include <Windows.h>


class OpenGLContext : public RenderContext
{
public:
	OpenGLContext(HWND windowHandle);

	void Init() override;
	void SetSwapInterval(int interval) override;

	void SwapBuffers() override;

	void Delete() override;


private:
	HWND windowHandle = NULL;
	HDC deviceContext = NULL;

	HGLRC context = NULL;
};