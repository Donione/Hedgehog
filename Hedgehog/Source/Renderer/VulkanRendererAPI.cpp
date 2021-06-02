#include <Renderer/VulkanRendererAPI.h>


namespace Hedge
{

void VulkanRendererAPI::Init(RenderContext* renderContext)
{
	this->renderContext = dynamic_cast<VulkanContext*>(renderContext);
}

void VulkanRendererAPI::Resize(int width, int height, bool fillViewport)
{
	// TODO the actuall resizing is not here yet, duh

	if (fillViewport)
	{
		SetViewport(0, 0, width, height);
		SetScissor(0, 0, width, height);
	}
}

void VulkanRendererAPI::SetViewport(int x, int y, int width, int height)
{
	viewport.offset.x = x;
	viewport.offset.y = y;
	viewport.extent.width = width;
	viewport.extent.height = height;
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
	renderContext->WaitForNextFrame();
}

void VulkanRendererAPI::BeginFrame()
{
	uint32_t swapchainImageIndex = renderContext->WaitForNextFrame();

	VkCommandBuffer commandBuffer = renderContext->mainCommandBuffer;

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


	//make a clear-color from frame number. This will flash with a 120*pi frame period.
	VkClearValue clearValue{};
	clearValue.color = { { clearColor.r, clearColor.g, clearColor.b, clearColor.a } };

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	rpInfo.renderPass = renderContext->renderPass;
	rpInfo.renderArea = viewport;
	rpInfo.framebuffer = renderContext->framebuffers[swapchainImageIndex];

	//connect clear values
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRendererAPI::EndFrame()
{
	VkCommandBuffer commandBuffer = renderContext->mainCommandBuffer;

	//finalize the render pass
	vkCmdEndRenderPass(commandBuffer);

	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		assert(false);
	}


	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &renderContext->presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &renderContext->renderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &commandBuffer;

	//submit command buffer to the queue and execute it.
	// renderFence will now block until the graphic commands finish execution
	if (vkQueueSubmit(renderContext->graphicsQueue, 1, &submit, renderContext->renderFence) != VK_SUCCESS)
	{
		assert(false);
	}

	renderContext->SwapBuffers();
}

void VulkanRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, unsigned int count, unsigned int offset)
{
}

} // namespace Hedge
