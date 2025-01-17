#include <Renderer/DirectX12RendererAPI.h>

#include <Renderer/RenderCommand.h>

#include <glm/gtc/type_ptr.hpp>
#include <backends/imgui_impl_dx12.h>


namespace Hedge
{

void DirectX12RendererAPI::Init(RenderContext* renderContext)
{
	this->renderContext = dynamic_cast<DirectX12Context*>(renderContext);
}

void DirectX12RendererAPI::Resize(int width, int height, bool fillViewport)
{
	if (width == viewport.Width
		&& height == viewport.Height)
	{
		return;
	}

	renderContext->WaitForLastSubmittedFrame();
	ImGui_ImplDX12_InvalidateDeviceObjects();
	renderContext->CleanupRenderTarget();
	renderContext->ResizeSwapChain(width, height);
	renderContext->ResizeDepthStencilBuffer(width, height);
	renderContext->CreateRenderTarget();
	ImGui_ImplDX12_CreateDeviceObjects();

	if (fillViewport)
	{
		SetViewport(0, 0, width, height);
		SetScissor(0, 0, width, height);
	}
}

void DirectX12RendererAPI::SetViewport(int x, int y, int width, int height)
{
	viewport = CD3DX12_VIEWPORT(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));
}

void DirectX12RendererAPI::SetScissor(int x, int y, int width, int height)
{
	scissorRect = CD3DX12_RECT(x, y, static_cast<LONG>(x + width), static_cast<LONG>(y + height));
}

void DirectX12RendererAPI::Begin()
{
	// Close the command list and execute it to begin the initial GPU setup.
	renderContext->g_pd3dCommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { renderContext->g_pd3dCommandList };
	renderContext->g_pd3dCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	UINT64 fenceValue = renderContext->g_fenceLastSignaledValue + 1;
	renderContext->g_pd3dCommandQueue->Signal(renderContext->g_fence, fenceValue);
	renderContext->g_fenceLastSignaledValue = fenceValue;
	renderContext->g_frameContext[0].FenceValue = fenceValue;
}

void DirectX12RendererAPI::End()
{
	// We have still some frames in-flight at this point
	// We need to wait for the GPU to finish before we let the destructors clean-up
	renderContext->WaitForLastSubmittedFrame();
}

void DirectX12RendererAPI::BeginFrame()
{
	// TODO ensure that BeginFrame and EndFrame are always called in pairs and BeginFrame is called first.
	frameCtxt = renderContext->WaitForNextFrameResources();
	backBufferIdx = renderContext->g_pSwapChain->GetCurrentBackBufferIndex();

	frameCtxt->CommandAllocator->Reset();
	// We're setting the pipeline state as null here, each vertexArray will set its own
	renderContext->g_pd3dCommandList->Reset(frameCtxt->CommandAllocator, nullptr);

	barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderContext->g_mainRenderTargetResource[backBufferIdx];
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	renderContext->g_pd3dCommandList->ResourceBarrier(1, &barrier);

	if (RenderCommand::GetDepthTest())
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(renderContext->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		renderContext->g_pd3dCommandList->OMSetRenderTargets(1, &renderContext->g_mainRenderTargetDescriptor[backBufferIdx], FALSE, &dsvHandle);
		renderContext->g_pd3dCommandList->ClearDepthStencilView(renderContext->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
																D3D12_CLEAR_FLAG_DEPTH,
																1.0f,
																0,
																0,
																nullptr);
	}
	else
	{
		renderContext->g_pd3dCommandList->OMSetRenderTargets(1, &renderContext->g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
	}

	renderContext->g_pd3dCommandList->ClearRenderTargetView(renderContext->g_mainRenderTargetDescriptor[backBufferIdx],
															glm::value_ptr(clearColor),
															0,
															NULL);
	renderContext->g_pd3dCommandList->RSSetViewports(1, &viewport);
	renderContext->g_pd3dCommandList->RSSetScissorRects(1, &scissorRect);
}

void DirectX12RendererAPI::EndFrame()
{
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	renderContext->g_pd3dCommandList->ResourceBarrier(1, &barrier);

	renderContext->g_pd3dCommandList->Close();

	ID3D12CommandList* commandLists[] = { (ID3D12CommandList*)renderContext->g_pd3dCommandList };
	renderContext->g_pd3dCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	renderContext->SwapBuffers();

	UINT64 fenceValue = renderContext->g_fenceLastSignaledValue + 1;
	renderContext->g_pd3dCommandQueue->Signal(renderContext->g_fence, fenceValue);
	renderContext->g_fenceLastSignaledValue = fenceValue;
	frameCtxt->FenceValue = fenceValue;
}

void DirectX12RendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray,
									   unsigned int count,
									   unsigned int offset)
{
	// TODO experiment with SV_InstanceID
	renderContext->g_pd3dCommandList->DrawIndexedInstanced(count > 0 ? count * 3 : vertexArray->GetIndexBuffer()->GetCount(),
														   vertexArray->GetInstanceCount(),
														   offset * 3,
														   0,
														   0);
}

} // namespace Hedge
