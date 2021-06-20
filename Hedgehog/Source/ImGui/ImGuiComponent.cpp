#include <ImGui/ImGuiComponent.h>

#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_vulkan.h>

#include <Renderer/Renderer.h>
#include <Renderer/DirectX12Context.h>
#include <Renderer/VulkanContext.h>

#include <cstdio>


static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}


namespace Hedge
{

ImGuiComponent::ImGuiComponent(HWND hwnd, RenderContext* renderContext)
	: renderContext(renderContext)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
	// 1st part of workaround for docked viewport layout
	// Docked windows use ChildBg for their background color or something
	// see https://github.com/ocornut/imgui/issues/2700
	// I'm not sure exactly how this works but this combination of WindowBg and ChildBg colors and the second part works
	// This is temporary anyways since the desired output will be accomplished by rendering to a texture and displaying that in the viewport window
	colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f); // same as ImGuiCol_WindowBg

	// Setup Platform/Renderer bindings
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		ImGui_ImplWin32_InitForOpenGL(hwnd);
		ImGui_ImplOpenGL3_Init("#version 460");
		break;

	case RendererAPI::API::DirectX12:
	{
		ImGui_ImplWin32_InitForDirectX12(hwnd);
		DirectX12Context* dx12renderContext = dynamic_cast<DirectX12Context*>(renderContext);
		CreateSRVDescHeap();
		ImGui_ImplDX12_Init(dx12renderContext->g_pd3dDevice,
							dx12renderContext->NUM_FRAMES_IN_FLIGHT,
							DXGI_FORMAT_R8G8B8A8_UNORM,
							SRVDescHeap,
							SRVDescHeap->GetCPUDescriptorHandleForHeapStart(),
							SRVDescHeap->GetGPUDescriptorHandleForHeapStart());
		break;
	}

	case RendererAPI::API::Vulkan:
	{
		ImGui_ImplWin32_InitForVulkan(hwnd);

		auto vulkanContext = dynamic_cast<VulkanContext*>(renderContext);

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vulkanContext->instance;
		init_info.PhysicalDevice = vulkanContext->chosenGPU;
		init_info.Device = vulkanContext->device;
		init_info.QueueFamily = vulkanContext->graphicsQueueFamily;
		init_info.Queue = vulkanContext->graphicsQueue;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = vulkanContext->descriptorPool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.ImageCount = vulkanContext->NUM_FRAMES_IN_FLIGHT;
		init_info.CheckVkResultFn = check_vk_result;

		ImGui_ImplVulkan_Init(&init_info, vulkanContext->renderPass);

		UploadFonts();

		break;
	}

	case RendererAPI::API::None:
	default:
		assert(false);
		break;
	}

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
}

void ImGuiComponent::BeginFrame()
{
	// Start the Dear ImGui frame
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		ImGui_ImplOpenGL3_NewFrame(); break;

	case RendererAPI::API::DirectX12:
		ImGui_ImplDX12_NewFrame(); break;

	case RendererAPI::API::Vulkan:
		ImGui_ImplVulkan_NewFrame(); break;

	case RendererAPI::API::None:
	default:
		assert(false);
		break;
	}

	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiComponent::EndFrame()
{
	// Rendering ImGui
	ImGui::Render();

	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		break;

	case RendererAPI::API::DirectX12:
	{
		DirectX12Context* dx12renderContext = dynamic_cast<DirectX12Context*>(renderContext);
		dx12renderContext->g_pd3dCommandList->SetDescriptorHeaps(1, &SRVDescHeap);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx12renderContext->g_pd3dCommandList);
		break;
	}

	case RendererAPI::API::Vulkan:
	{
		VulkanContext* vulkanContext = dynamic_cast<VulkanContext*>(renderContext);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vulkanContext->commandBuffers[vulkanContext->frameInFlightIndex]);
		break;
	}

	case RendererAPI::API::None:
	default:
		assert(false);
		break;
	}

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		renderContext->MakeCurrent();
	}
}

void ImGuiComponent::CreateSRVDescHeap()
{
	DirectX12Context* dx12renderContext = dynamic_cast<DirectX12Context*>(renderContext);
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	dx12renderContext->g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&SRVDescHeap));
}

void ImGuiComponent::UploadFonts()
{
	auto vulkanContext = dynamic_cast<VulkanContext*>(renderContext);

	// Use any command queue
	VkCommandPool command_pool = vulkanContext->commandPool;
	VkCommandBuffer command_buffer = vulkanContext->commandBuffers[vulkanContext->frameInFlightIndex];

	auto err = vkResetCommandPool(vulkanContext->device, command_pool, 0);
	check_vk_result(err);
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	err = vkBeginCommandBuffer(command_buffer, &begin_info);
	check_vk_result(err);

	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &command_buffer;
	err = vkEndCommandBuffer(command_buffer);
	check_vk_result(err);
	err = vkQueueSubmit(vulkanContext->graphicsQueue, 1, &end_info, VK_NULL_HANDLE);
	check_vk_result(err);

	err = vkDeviceWaitIdle(vulkanContext->device);
	check_vk_result(err);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

ImGuiComponent::~ImGuiComponent()
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		ImGui_ImplOpenGL3_Shutdown(); break;

	case RendererAPI::API::DirectX12:
		ImGui_ImplDX12_Shutdown(); break;

	case RendererAPI::API::Vulkan:
		ImGui_ImplVulkan_Shutdown(); break;

	case RendererAPI::API::None:
	default:
		assert(false);
		break;
	}

	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (SRVDescHeap) { SRVDescHeap->Release(); SRVDescHeap = nullptr; }

	if (descriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(dynamic_cast<VulkanContext*>(renderContext)->device, descriptorPool, nullptr);
	}
}

} // namespace Hedge
