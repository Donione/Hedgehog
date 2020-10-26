#pragma once

class RenderContext
{
public:
	virtual void Init() = 0;
	virtual void SetSwapInterval(int interval) = 0;

	virtual void SwapBuffers() = 0;

	virtual void Delete() = 0;
};
