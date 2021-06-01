#pragma once

#include <Renderer/RenderContext.h>

#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <vector>

// This implementation is directly based (copy-pasted from) on VulkanGuide (https://vkguide.dev)


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

	VkInstance instance; // Vulkan library handle
	VkDebugUtilsMessengerEXT debugMessenger; // Vulkan debug output handle
	VkPhysicalDevice chosenGPU; // GPU chosen as the default device
	VkDevice device; // Vulkan device for commands
	VkSurfaceKHR surface; // Vulkan window surface

	VkSwapchainKHR swapchain;

	// image format expected by the windowing system
	VkFormat swapchainImageFormat;
	
	//array of images from the swapchain
	std::vector<VkImage> swapchainImages;

	//array of image-views from the swapchain
	std::vector<VkImageView> swapchainImageViews;

};

}  // namespace Hedge
