#pragma once

#include <Renderer/RenderContext.h>

#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

#include <vector>

// This implementation is directly based (copy-pasted from) on VulkanGuide (https://vkguide.dev)
// and Vulkan Tutorial (https://vulkan-tutorial.com)


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

	friend class VulkanRendererAPI;
	friend class ImGuiComponent;
	friend class VulkanShader;
	friend class VulkanVertexArray;

private:
	vkb::Instance CreateInstance();
	void CreateDevice(vkb::Instance& vkbInst);
	void CreateSwapChain(unsigned int width, unsigned int height);
	void CreateCommandBuffers();
	void CreateRenderPass();
	void CreateFrameBuffers(unsigned int width, unsigned int height);
	void CreateSyncObjects();

	void DestroySwapChain();
	void DestroyFrameBuffers();
	void DestroySyncObjects();

	void ResizeSwapChain(unsigned int width, unsigned int height);

	uint32_t WaitForNextFrame();


private:
	static int const NUM_FRAMES_IN_FLIGHT = 3;

	HWND windowHandle = NULL;
	int swapInterval = 0;
	uint32_t swapChainImageIndex = 0;
	int frameInFlightIndex = 0;

	VkInstance instance; // Vulkan library handle
	VkDebugUtilsMessengerEXT debugMessenger; // Vulkan debug output handle
	VkPhysicalDevice chosenGPU; // GPU chosen as the default device
	VkDevice device; // Vulkan device for commands
	VkSurfaceKHR surface; // Vulkan window surface

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	// image format expected by the windowing system
	VkFormat swapchainImageFormat;
	//array of images from the swapchain
	std::vector<VkImage> swapchainImages;
	//array of image-views from the swapchain
	std::vector<VkImageView> swapchainImageViews;

	VkQueue graphicsQueue; //queue we will submit to
	uint32_t graphicsQueueFamily; //family of that queue

	VkCommandPool commandPool; //the command pool for our commands
	std::vector<VkCommandBuffer> commandBuffers; //the buffers we will record into

	VkRenderPass renderPass;

	std::vector<VkFramebuffer> framebuffers;

	// Semaphores are used for GPU-GPU synchronization
	std::vector<VkSemaphore> presentSemaphores;
	std::vector<VkSemaphore> renderSemaphores;
	// Fences are used for CPU-GPU synchronization
	std::vector<VkFence> frameInFlightFences;
	std::vector<VkFence> swapChainImageFences;
};

}  // namespace Hedge
