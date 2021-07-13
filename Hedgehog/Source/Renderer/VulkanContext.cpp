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
	CreadeDepthImage(Application::GetInstance().GetWindow().GetWidth(),
					 Application::GetInstance().GetWindow().GetHeight());
	CreateCommandBuffers();
	CreateRenderPass();
	CreateFrameBuffers(Application::GetInstance().GetWindow().GetWidth(),
					   Application::GetInstance().GetWindow().GetHeight());
	CreateSyncObjects();
	CreateDescriptorPool();
}

VulkanContext::~VulkanContext()
{
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	DestroySyncObjects();
	vkDestroyImageView(device, depthImageView, nullptr);
	DestroyVulkanImage(depthImage, depthImageMemory);
	DestroySwapChain();
	DestroyFrameBuffers();
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkb::destroy_debug_utils_messenger(instance, debugMessenger);
	vkDestroyInstance(instance, nullptr);
}

void VulkanContext::SetSwapInterval(int interval)
{
	if (interval == swapInterval)
	{
		return;
	}

	swapInterval = interval;

	// Rebuild the swapchain with new present mode
	DestroySwapChain();
	DestroyFrameBuffers();

	unsigned int width = Application::GetInstance().GetWindow().GetWidth();
	unsigned int height = Application::GetInstance().GetWindow().GetHeight();
	CreateSwapChain(width, height);
	CreateFrameBuffers(width, height);
}


void VulkanContext::SwapBuffers()
{
	// this will put the image we just rendered into the visible window.
	// we want to wait on the renderSemaphore for that, 
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &renderSemaphores[frameInFlightIndex];
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapChainImageIndex;

	VkResult result = vkQueuePresentKHR(graphicsQueue, &presentInfo);
	if (result != VK_SUCCESS)
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
	createInfo.hinstance = Application::GetInstance().GetHInstance();

	if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
	{
		assert(false);
	}

	//use vkbootstrap to select a GPU. 
	//We want a GPU that can write to the SDL surface and supports Vulkan 1.1
	vkb::PhysicalDeviceSelector selector{ vkbInst };
	VkPhysicalDeviceFeatures features{};
	features.fillModeNonSolid = true;
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_required_features(features)
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
		//.use_default_format_selection()
		.set_desired_format({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(swapInterval == 0 ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		.set_desired_extent(width, height)
		.build()
		.value();

	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();
	swapchainImageFormat = vkbSwapchain.image_format;
}

void VulkanContext::CreadeDepthImage(unsigned int width, unsigned int height)
{
	depthFormat = VK_FORMAT_D32_SFLOAT;

	CreateImage(width,
				height,
				VK_IMAGE_TYPE_2D,
				depthFormat,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				depthImage,
				depthImageMemory);
	depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
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

	commandBuffers.resize(swapchainImages.size());
	for (int i = 0; i < swapchainImages.size(); i++)
	{
		if (vkAllocateCommandBuffers(device, &cmdAllocInfo, &commandBuffers[i]) != VK_SUCCESS)
		{
			assert(false);
		}
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


	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::vector<VkAttachmentDescription> attachments = { color_attachment, depthAttachment };
	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//connect the color attachment to the info
	render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	render_pass_info.pAttachments = attachments.data();
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

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
	fb_info.renderPass = renderPass;
	fb_info.width = width;
	fb_info.height = height;
	fb_info.layers = 1;

	//grab how many images we have in the swapchain
	const size_t swapchain_imagecount = swapchainImages.size();
	framebuffers.resize(swapchain_imagecount);

	//create framebuffers for each of the swapchain image views
	for (size_t i = 0; i < swapchain_imagecount; i++)
	{
		std::vector<VkImageView> attachments = { swapchainImageViews[i], depthImageView };
		fb_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		fb_info.pAttachments = attachments.data();

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

	frameInFlightFences.resize(NUM_FRAMES_IN_FLIGHT);
	for (int i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateFence(device, &fenceCreateInfo, nullptr, &frameInFlightFences[i]) != VK_SUCCESS)
		{
			assert(false);
		}
	}
	swapChainImageFences.resize(swapchainImages.size(), VK_NULL_HANDLE);

	//for the semaphores we don't need any flags
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	presentSemaphores.resize(NUM_FRAMES_IN_FLIGHT);
	renderSemaphores.resize(NUM_FRAMES_IN_FLIGHT);
	for (int i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphores[i]) != VK_SUCCESS)
		{
			assert(false);
		}
		if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphores[i]) != VK_SUCCESS)
		{
			assert(false);
		}
	}
}

void VulkanContext::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSizes[] =
	{
		// type,                                     descriptorCount
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolInfo.maxSets = 2 * 1000;
	descriptorPoolInfo.poolSizeCount = 2;
	descriptorPoolInfo.pPoolSizes = poolSizes;

	if (vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		assert(false);
	}
}

void VulkanContext::DestroySwapChain()
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void VulkanContext::DestroyFrameBuffers()
{
	for (int i = 0; i < swapchainImageViews.size(); i++)
	{
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
		vkDestroyImageView(device, swapchainImageViews[i], nullptr);
	}
}

void VulkanContext::DestroySyncObjects()
{
	for (int i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, presentSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderSemaphores[i], nullptr);
		vkDestroyFence(device, frameInFlightFences[i], nullptr);
	}
}

void VulkanContext::ResizeSwapChain(unsigned int width, unsigned int height)
{
	vkDestroyImageView(device, depthImageView, nullptr);
	DestroyVulkanImage(depthImage, depthImageMemory);
	DestroySwapChain();
	DestroyFrameBuffers();

	CreateSwapChain(width, height);
	CreadeDepthImage(width, height);
	CreateFrameBuffers(width, height);
}

uint32_t VulkanContext::WaitForNextFrame()
{
	//wait until the GPU has finished rendering the last frame
	// I believe the following wait on fence is needed only when NUM_FRAMES_IN_FLIGHT < swapchainImages.size()
	vkWaitForFences(device, 1, &frameInFlightFences[frameInFlightIndex], true, UINT64_MAX);

	//request image from the swapchain
	uint32_t swapChainImageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentSemaphores[frameInFlightIndex], nullptr, &swapChainImageIndex);
	if (result != VK_SUCCESS)
	{
		assert(false);
	}

	// Make sure that the image/commandBuffer we just acquired is ready to be used
	if (swapChainImageFences[swapChainImageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(device, 1, &swapChainImageFences[swapChainImageIndex], true, UINT64_MAX);
	}

	// Assign a fence for the next image
	swapChainImageFences[swapChainImageIndex] = frameInFlightFences[frameInFlightIndex];

	this->swapChainImageIndex = swapChainImageIndex;

	return swapChainImageIndex;
}

uint32_t VulkanContext::FindMemoryType(uint32_t requiredType, VkMemoryPropertyFlags requiredProperties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(chosenGPU, &memProperties);

	for (uint32_t memoryType = 0; memoryType < memProperties.memoryTypeCount; memoryType++)
	{
		bool isRequiredType = (1 << memoryType) & requiredType;
		bool hasRequiredProperties = (memProperties.memoryTypes[memoryType].propertyFlags & requiredProperties) == requiredProperties;

		if (isRequiredType && hasRequiredProperties)
		{
			return memoryType;
		}
	}

	// We didn't find any suitable memory type, just end it
	assert(false);
	return 0;
}

void VulkanContext::CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* buffer)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, buffer) != VK_SUCCESS)
	{
		assert(false);
	}
}

void VulkanContext::AllocateMemory(VkBuffer buffer, VkMemoryPropertyFlags requiredProperties, VkDeviceMemory* bufferMemory)
{
	// Query the memory requirements for the vertex buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	// Allocate memory for the vertex buffer
	// TODO We're using CPU visible buffer for now, will be changed to use staging buffers
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits,
											   requiredProperties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory) != VK_SUCCESS)
	{
		assert(false);
	}
}

void VulkanContext::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	// TODO temporary create a new commandBuffer and submit it here
	// We want to move it to RendererAPI::Begin() so we can submit multiple transfers and all that good stuff
	// Possibly creating a separate command pool amd queue dedicate for transfers

	// Create a one-shot command buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// Record a transfer command
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	// Submit the command buffer and wait for completion
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	// Clean up the command buffer
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanContext::CreateBuffer(VkDeviceSize size,
								 VkBufferUsageFlags usage,
								 VkMemoryPropertyFlags requiredProperties,
								 VkBuffer* buffer,
								 VkDeviceMemory* bufferMemory)
{
	CreateVulkanBuffer(size, usage, buffer);
	AllocateMemory(*buffer, requiredProperties, bufferMemory);
	vkBindBufferMemory(device,
					   *buffer,
					   *bufferMemory,
					   0); // offset into the block of memory, must be aligned according to the memRequirements.alignment
}

void VulkanContext::CreateStagedBuffer(VkBufferUsageFlags usage,
									   void* data,
									   VkDeviceSize size,
									   VkBuffer* buffer,
									   VkDeviceMemory* bufferMemory)
{
	// Create a staging buffer
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	CreateBuffer(size,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 &stagingBuffer,
				 &stagingBufferMemory);

	// Copy data to the staging buffer
	void* mappedData;
	vkMapMemory(device, stagingBufferMemory, 0, size, 0, &mappedData);
	memcpy(mappedData, data, size);
	vkUnmapMemory(device, stagingBufferMemory);

	// Create buffer on GPU
	CreateBuffer(size,
				 VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 buffer,
				 bufferMemory);

	CopyBuffer(stagingBuffer, *buffer, size);

	// Clean up the staging buffer
	DestoyVulkanBuffer(stagingBuffer, stagingBufferMemory);
}

void VulkanContext::CreateVulkanImage(unsigned int width,
									  unsigned int height,
									  VkImageType type,
									  VkFormat format,
									  VkImageUsageFlags usage,
									  VkImage& image)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = type;
	imageInfo.format = format;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		assert(false);
	}
}

void VulkanContext::AllocateImageMemory(VkImage image,
										VkMemoryPropertyFlags requiredProperties,
										VkDeviceMemory& imageMemory)
{
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, requiredProperties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		assert(false);
	}

	vkBindImageMemory(device, image, imageMemory, 0);
}

void VulkanContext::CreateImage(unsigned int width,
								unsigned int height,
								VkImageType type,
								VkFormat format,
								VkImageUsageFlags usage,
								VkImage& image,
								VkDeviceMemory& imageMemory)
{
	CreateVulkanImage(width, height, type, format, usage, image);
	AllocateImageMemory(image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageMemory);
}

VkImageView VulkanContext::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		assert(false);
	}

	return imageView;
}

void VulkanContext::DestoyVulkanBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory)
{
	if (bufferMemory != VK_NULL_HANDLE) vkFreeMemory(device, bufferMemory, nullptr);
	if (buffer != VK_NULL_HANDLE) vkDestroyBuffer(device, buffer, nullptr);
}

void VulkanContext::DestroyVulkanImage(VkImage image, VkDeviceMemory imageMemory)
{
	if (imageMemory != VK_NULL_HANDLE) vkFreeMemory(device, imageMemory, nullptr);
	if (image != VK_NULL_HANDLE) vkDestroyImage(device, image, nullptr);
}

} // namespace Hedge
