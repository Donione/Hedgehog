#pragma once

#include <Renderer/RenderContext.h>

#include <Windows.h>


namespace Hedge
{

class OpenGLContext : public RenderContext
{
public:
	OpenGLContext(HWND windowHandle);
	virtual ~OpenGLContext() override;

	void SetSwapInterval(int interval) override;
	void MakeCurrent() override;
	void SwapBuffers() override;


private:
	HWND windowHandle = NULL;
	HDC deviceContext = NULL;

	HGLRC context = NULL;
};

} // namespace Hedge