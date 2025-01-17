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

	void SetSwapInterval(int interval) override;
	void MakeCurrent() override { /* Do Nothing */ }
	void SwapBuffers() override;

	friend class VulkanRendererAPI;
	friend class ImGuiComponent;
	friend class VulkanShader;
	friend class VulkanVertexArray;
	friend class VulkanVertexBuffer;
	friend class VulkanIndexBuffer;

private:
	vkb::Instance CreateInstance();
	void CreateDevice(vkb::Instance& vkbInst);
	void CreateSwapChain(unsigned int width, unsigned int height);
	void CreadeDepthImage(unsigned int width, unsigned int height);
	void CreateCommandBuffers();
	void CreateRenderPass();
	void CreateFrameBuffers(unsigned int width, unsigned int height);
	void CreateSyncObjects();
	void CreateDescriptorPool();

	void DestroySwapChain();
	void DestroyFrameBuffers();
	void DestroySyncObjects();

	void ResizeSwapChain(unsigned int width, unsigned int height);

	uint32_t WaitForNextFrame();

	uint32_t FindMemoryType(uint32_t requiredType, VkMemoryPropertyFlags requiredProperties);
	void CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* buffer);
	void AllocateMemory(VkBuffer buffer, VkMemoryPropertyFlags requiredProperties, VkDeviceMemory* bufferMemory);
	VkCommandBuffer BeginSingleUseCommands();
	void EndSingleUseCommands(VkCommandBuffer commandBuffer);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CreateBuffer(VkDeviceSize size,
					  VkBufferUsageFlags usage,
					  VkMemoryPropertyFlags requiredProperties,
					  VkBuffer* buffer,
					  VkDeviceMemory* bufferMemory);
	void CreateStagedBuffer(VkBufferUsageFlags usage,
							void* data,
							VkDeviceSize size,
							VkBuffer* buffer,
							VkDeviceMemory* bufferMemory);

	void CreateVulkanImage(unsigned int width,
						   unsigned int height,
						   VkImageType type,
						   VkFormat format,
						   VkImageUsageFlags usage,
						   VkImage& image);
	void AllocateImageMemory(VkImage image,
							 VkMemoryPropertyFlags requiredProperties,
							 VkDeviceMemory& imageMemory);
	void CreateImage(unsigned int width,
					 unsigned int height,
					 VkImageType type,
					 VkFormat format,
					 VkImageUsageFlags usage,
					 VkImage& image,
					 VkDeviceMemory& imageMemory);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void DestoyVulkanBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory);
	void DestroyVulkanImage(VkImage image, VkDeviceMemory imageMemory);


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

	VkFormat depthFormat;
	VkImage depthImage = VK_NULL_HANDLE;
	VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;

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

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
};

}  // namespace Hedge
