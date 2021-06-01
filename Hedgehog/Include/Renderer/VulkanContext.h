#pragma once

#include <Renderer/RenderContext.h>

#include <Windows.h>

#include <vulkan/vulkan.h>


namespace Hedge
{

class VulkanContext : public RenderContext
{
public:
	VulkanContext(HWND windowHandle);
	virtual ~VulkanContext() override;

	void SetSwapInterval(int interval) override { swapInterval = interval; }
	void MakeCurrent() override { /* Do Nothing */ }
	void SwapBuffers() override;

private:
	HWND windowHandle = NULL;

	int swapInterval = 0;
};

}  // namespace Hedge
