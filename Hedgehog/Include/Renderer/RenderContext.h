#pragma once


namespace Hedge
{

class RenderContext
{
public:
	virtual ~RenderContext() = default;

	virtual void SetSwapInterval(int interval) = 0;
	virtual void MakeCurrent() = 0;
	virtual void SwapBuffers() = 0;
};

} // namespace Hedge
