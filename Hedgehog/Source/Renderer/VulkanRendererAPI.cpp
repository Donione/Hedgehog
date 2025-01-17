#include <Application/Application.h>

#include <Renderer/VulkanRendererAPI.h>

#include <backends/imgui_impl_vulkan.h>


namespace Hedge
{

void VulkanRendererAPI::Init(RenderContext* renderContext)
{
	this->renderContext = dynamic_cast<VulkanContext*>(renderContext);
}

void VulkanRendererAPI::Resize(int width, int height, bool fillViewport)
{
	if (width == viewport.width
		&& height == viewport.height)
	{
		return;
	}

	vkDeviceWaitIdle(renderContext->device);

	renderContext->ResizeSwapChain(width, height);

	if (fillViewport)
	{
		SetViewport(0, 0, width, height);
		SetScissor(0, 0, width, height);
	}
}

void VulkanRendererAPI::SetViewport(int x, int y, int width, int height)
{
	viewport.x = (float)x;
	viewport.y = (float)y;
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
}

void VulkanRendererAPI::SetScissor(int x, int y, int width, int height)
{
	scissor.offset.x = x;
	scissor.offset.y = y;
	scissor.extent.width = width;
	scissor.extent.height = height;
}

void VulkanRendererAPI::End()
{
	vkDeviceWaitIdle(renderContext->device);
}

void VulkanRendererAPI::BeginFrame()
{
	uint32_t swapchainImageIndex = renderContext->WaitForNextFrame();

	VkCommandBuffer commandBuffer = renderContext->commandBuffers[renderContext->swapChainImageIndex];

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS)
	{
		assert(false);
	}

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo) != VK_SUCCESS)
	{
		assert(false);
	}


	std::vector<VkClearValue> clearValues;
	clearValues.resize(2, {});
	clearValues[0].color = { { clearColor.r, clearColor.g, clearColor.b, clearColor.a } };
	clearValues[1].depthStencil = { 1.0, 0 };

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	rpInfo.renderPass = renderContext->renderPass;

	// Set the render area to the entire window/swap chain size, so the ImGui renders properly since it uses the same render pass
	// but will (could) render outside of the main window viewport panel
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent.width = Application::GetInstance().GetWindow().GetWidth();
	rpInfo.renderArea.extent.height = Application::GetInstance().GetWindow().GetHeight();
	rpInfo.framebuffer = renderContext->framebuffers[swapchainImageIndex];

	//connect clear values
	rpInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	rpInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRendererAPI::EndFrame()
{
	VkCommandBuffer commandBuffer = renderContext->commandBuffers[renderContext->swapChainImageIndex];

	//finalize the render pass
	vkCmdEndRenderPass(commandBuffer);

	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		assert(false);
	}


	//prepare the submission to the queue.
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	// we want to wait on the presentSemaphore, as that semaphore is signaled when the swapchain is ready
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &renderContext->presentSemaphores[renderContext->frameInFlightIndex];

	// we will signal the renderSemaphore, to signal that rendering has finished
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &renderContext->renderSemaphores[renderContext->frameInFlightIndex];

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &commandBuffer;

	// submit command buffer to the queue and execute it.
	// renderFence will now block until the graphic commands finish execution
	vkResetFences(renderContext->device, 1, &renderContext->frameInFlightFences[renderContext->frameInFlightIndex]);
	if (vkQueueSubmit(renderContext->graphicsQueue, 1, &submit, renderContext->frameInFlightFences[renderContext->frameInFlightIndex]) != VK_SUCCESS)
	{
		assert(false);
	}

	renderContext->SwapBuffers();

	renderContext->frameInFlightIndex = (renderContext->frameInFlightIndex + 1) % renderContext->NUM_FRAMES_IN_FLIGHT;
}

void VulkanRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, unsigned int count, unsigned int offset)
{
	vkCmdDrawIndexed(renderContext->commandBuffers[renderContext->swapChainImageIndex],
					 count > 0 ? count * 3 : vertexArray->GetIndexBuffer()->GetCount(),
					 vertexArray->GetInstanceCount(), 
					 offset * 3,
					 0, // vertexOffset
					 0); // firstInstance
}

} // namespace Hedge
