#include <Renderer/DirectX12RendererAPI.h>

#include <glm/gtc/type_ptr.hpp>
#include <imgui_impl_dx12.h>


void DirectX12RendererAPI::Init(RenderContext* renderContext)
{
	this->renderContext = dynamic_cast<DirectX12Context*>(renderContext);
}

void DirectX12RendererAPI::SetViewport(int width, int height)
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
	renderContext->CreateRenderTarget();
	ImGui_ImplDX12_CreateDeviceObjects();

	viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
	scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
}

void DirectX12RendererAPI::BeginFrame()
{
	// TODO ensure that BeginFrame and EndFrame are always called in pairs and BeginFrame is called first.
	frameCtxt = frameCtxt = renderContext->WaitForNextFrameResources();
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

	renderContext->g_pd3dCommandList->OMSetRenderTargets(1, &renderContext->g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
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
