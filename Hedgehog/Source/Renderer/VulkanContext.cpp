#include <Renderer/VulkanContext.h>

#include <VkBootstrap.h>

#include <Application/Application.h>


namespace Hedge
{

VulkanContext::VulkanContext(HWND windowHandle)
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
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
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


	vkb::SwapchainBuilder swapchainBuilder{ chosenGPU, device, surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(Application::GetInstance().GetWindow().GetWidth(), Application::GetInstance().GetWindow().GetHeight())
		.build()
		.value();

	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();

	swapchainImageFormat = vkbSwapchain.image_format;
}

VulkanContext::~VulkanContext()
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);

	//destroy swapchain resources
	for (int i = 0; i < swapchainImageViews.size(); i++) {

		vkDestroyImageView(device, swapchainImageViews[i], nullptr);
	}

	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkb::destroy_debug_utils_messenger(instance, debugMessenger);
	vkDestroyInstance(instance, nullptr);
}

void VulkanContext::SwapBuffers()
{
}

} // namespace Hedge
