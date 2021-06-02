#include <Renderer/VulkanContext.h>

#include <Application/Application.h>


namespace Hedge
{

VulkanContext::VulkanContext(HWND windowHandle)
{
	this->windowHandle = windowHandle;

	vkb::Instance vkbInst = CreateInstance();
	CreateDevice(vkbInst);
	CreateSwapChain(Application::GetInstance().GetWindow().GetWidth(),
					Application::GetInstance().GetWindow().GetHeight());
	CreateCommandBuffers();
	CreateRenderPass();
	CreateFrameBuffers(Application::GetInstance().GetWindow().GetWidth(),
					   Application::GetInstance().GetWindow().GetHeight());
	CreateSyncObjects();
}

VulkanContext::~VulkanContext()
{
	DestroySyncObjects();
	vkDestroyCommandPool(device, commandPool, nullptr);
	DestroySwapChain();
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkb::destroy_debug_utils_messenger(instance, debugMessenger);
	vkDestroyInstance(instance, nullptr);
}


void VulkanContext::SwapBuffers()
{
	// this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &frameIndex;

	if (vkQueuePresentKHR(graphicsQueue, &presentInfo) != VK_SUCCESS)
	{
		assert(false);
	}
}


vkb::Instance VulkanContext::CreateInstance()
{
	vkb::InstanceBuilder builder;

	//make the Vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(true)
		.require_api_version(1, 1, 0)
		.use_default_debug_messenger()
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	//store the instance 
	instance = vkb_inst.instance;
	//store the debug messenger
	debugMessenger = vkb_inst.debug_messenger;

	return vkb_inst;
}

void VulkanContext::CreateDevice(vkb::Instance& vkbInst)
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = windowHandle;
	createInfo.hinstance = GetModuleHandle(nullptr); // TODO get the instance from Application

	if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
	{
		assert(false);
	}

	//use vkbootstrap to select a GPU. 
	//We want a GPU that can write to the SDL surface and supports Vulkan 1.1
	vkb::PhysicalDeviceSelector selector{ vkbInst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(surface)
		.select()
		.value();

	//create the final Vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a Vulkan application
	device = vkbDevice.device;
	chosenGPU = physicalDevice.physical_device;

	// use vkbootstrap to get a Graphics queue
	graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanContext::CreateSwapChain(unsigned int width, unsigned int height)
{
	vkb::SwapchainBuilder swapchainBuilder{ chosenGPU, device, surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.build()
		.value();

	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();

	swapchainImageFormat = vkbSwapchain.image_format;
}

void VulkanContext::CreateCommandBuffers()
{
	//create a command pool for commands submitted to the graphics queue.	
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;

	//the command pool will be one that can submit graphics commands
	commandPoolInfo.queueFamilyIndex = graphicsQueueFamily;
	//we also want the pool to allow for resetting of individual command buffers
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		assert(false);
	}


	//allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo = {};
	cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocInfo.pNext = nullptr;

	//commands will be made from our commandPool
	cmdAllocInfo.commandPool = commandPool;
	//we will allocate 1 command buffer
	cmdAllocInfo.commandBufferCount = 1;
	// command level is Primary
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(device, &cmdAllocInfo, &mainCommandBuffer) != VK_SUCCESS)
	{
		assert(false);
	}
}

void VulkanContext::CreateRenderPass()
{
	// the renderpass will use this color attachment.
	VkAttachmentDescription color_attachment = {};

	//the attachment will have the format needed by the swapchain
	color_attachment.format = swapchainImageFormat;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// we Clear when this attachment is loaded
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// we keep the attachment stored when the renderpass ends
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//we don't care about stencil
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//we don't know or care about the starting layout of the attachment
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//after the renderpass ends, the image has to be on a layout ready for display
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	//connect the color attachment to the info
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	if (vkCreateRenderPass(device, &render_pass_info, nullptr, &renderPass) != VK_SUCCESS)
	{
		assert(false);
	}
}

void VulkanContext::CreateFrameBuffers(unsigned int width, unsigned int height)
{
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = nullptr;

	fb_info.renderPass = renderPass;
	fb_info.attachmentCount = 1;
	fb_info.width = width;
	fb_info.height = height;
	fb_info.layers = 1;

	//grab how many images we have in the swapchain
	const size_t swapchain_imagecount = swapchainImages.size();
	framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	//create framebuffers for each of the swapchain image views
	for (size_t i = 0; i < swapchain_imagecount; i++) {

		fb_info.pAttachments = &swapchainImageViews[i];
		if (vkCreateFramebuffer(device, &fb_info, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			assert(false);
		}
	}
}

void VulkanContext::CreateSyncObjects()
{
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = nullptr;

	//we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence) != VK_SUCCESS)
	{
		assert(false);
	}

	//for the semaphores we don't need any flags
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore) != VK_SUCCESS)
	{
		assert(false);
	}
	if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphore) != VK_SUCCESS)
	{
		assert(false);
	}
}

void VulkanContext::DestroySwapChain()
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);

	//destroy the main renderpass
	vkDestroyRenderPass(device, renderPass, nullptr);

	//destroy swapchain resources
	for (int i = 0; i < swapchainImageViews.size(); i++)
	{
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
		vkDestroyImageView(device, swapchainImageViews[i], nullptr);
	}
}

void VulkanContext::DestroySyncObjects()
{
	vkDestroySemaphore(device, presentSemaphore, nullptr);
	vkDestroySemaphore(device, renderSemaphore, nullptr);
	vkDestroyFence(device, renderFence, nullptr);
}

void VulkanContext::ResizeSwapChain(unsigned int width, unsigned int height)
{
	DestroySwapChain();
	CreateSwapChain(width, height);
}

uint32_t VulkanContext::WaitForNextFrame()
{
	//wait until the GPU has finished rendering the last frame. Timeout of 1 second
	vkWaitForFences(device, 1, &renderFence, true, 1000000000);
	vkResetFences(device, 1, &renderFence);

	//request image from the swapchain, one second timeout
	uint32_t swapchainImageIndex;
	if (vkAcquireNextImageKHR(device, swapchain, 1000000000, presentSemaphore, nullptr, &swapchainImageIndex) != VK_SUCCESS)
	{
		assert(false);
	}

	frameIndex = swapchainImageIndex;

	return swapchainImageIndex;
}

} // namespace Hedge
